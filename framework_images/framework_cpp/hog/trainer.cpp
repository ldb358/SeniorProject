#include "trainer.h"
#include<unistd.h>

using namespace cv;
using namespace std;


float hn_train(DsReader &dataset, LinearSVM *svm, vector< vector<float> > &training_data, 
		    vector<float> &labels, string type, float scale, float rects){
    struct DsImage img_data;
    dataset.reset();
    vector<float> support_vector;
    svm->getSupportVector(support_vector);
    HOGDescriptor gdesc;
    gdesc.setSVMDetector(support_vector);

    int pos_image = 0;
    int cur = 0;
    while(dataset.has_next()){
	//increment the image counter
	cur++;
	//load the next image info into the img_data struct
	dataset.next(img_data);
	//load the image
	Mat img = imread(img_data.path, 0);	
	//if the image is empty(it doesnt exist) skip it
	if(img.cols == 0) 
		continue;
	//upload the image to the gpu for detection
	//create the vector to hold the results
	vector<Rect> matches;
	//perform the multiscale match(on the gpu)
	gdesc.detectMultiScale(img, matches, 0, Size(), Size(0, 0), scale, rects);	
	
	//set the default state to the image should be negative
	for(int i=0; i < matches.size(); ++i){
	    int ispos = 0;
	    for(int n=0; n < img_data.tags.size() ; ++n){
		string cls = dataset.get_class(img_data.tags[n].clss);
		if(cls == type)
		    continue;
		int width = dataset.tag_width()*img_data.tags[n].scale;
		int height = dataset.tag_height()*img_data.tags[n].scale;	
		Rect tag(img_data.tags[n].pos.x, img_data.tags[n].pos.y, 
			    width, height);	
		Rect overlap = matches[i] & tag;
		if(((float)overlap.area())/tag.area() > .5){
		    ispos = 1;
		}
	    }
	    //first check if this tag is even the right type of object
	    if(!ispos){	
		vector<float> ders;
		matches[i].width = min(img.cols-matches[i].x, matches[i].width);
		matches[i].height = min(img.rows-matches[i].y, matches[i].height);
		Mat submat;
		resize(img(matches[i]), submat, Size(64,128));
		gdesc.compute(submat, ders);	
		training_data.push_back(ders);
		labels.push_back(-1.0);
	    }else{
		pos_image++;
	    }
	}
	cout << "\rTraining with image " << cur << "/" << dataset.size() << flush;
    }
    cout << "test" << endl;
    return pos_image/dataset.size();
}


int main(int argc, char**argv){
    double c = 100;
    double epi = 1e-6;
    int iter = 450;
    double overlap_thresh = .5;
    double scale = 1.10;
    int rects = 5;
    int opt;
    while((opt = getopt(argc-4, &argv[3], "c:e:i:o:s:r:")) != -1){
    	switch(opt){
	    case 'c':
		c = atof(optarg);
	    break;
	    case 'e':
		epi = atof(optarg);
	    break;
	    case 'i':
		iter = atoi(optarg);
	    break;
	    case 'o':
		overlap_thresh = atof(optarg);
		if(overlap_thresh > 1 || overlap_thresh < 0){
		    cout << "Overlap thresh must be between 0 and 1..." << endl;
		    return 1;
		}
	    break;
	    case 's':
		scale = atof(optarg);
	    break;
	    case 'r':
		rects =  atoi(optarg);
	    break;	
	}
    }
    if(argc < 4){
	cout << "usage: [json file for tags] [file with list of images for training ] [file to save model to] [flags]" << endl
	    << "flags:" << endl
	    << "-c [double]: set the c value for the svm" << endl
	    << "-e [double]: set the epsilon for the svm" << endl
	    << "-i [double]: set the number of interations that we can train for the svm" << endl
	    << "-o [double < 1] set the amount that a rectangle has to overlap with a tag to be a match" << endl
	    << "-s [double]: How much to scale the image for multdetect" << endl
	    << "-r [int]: set the number of rects to justify a match" << endl
	    << endl;
	return -1;
    }
    cout << "GPU Count:" << gpu::getCudaEnabledDeviceCount() << endl;
    FileReader file(argv[2]);
    if(!file.is_open()){
	cout << "File couldn't be opened... exiting.";
	return 1;
    }

    //arguments
    int step_size = STEP_SIZE;
    string type = CLASS;

    // save our descriptors in the object(since we are only calculating one at a time
    HOGDescriptor desc;
    //output file to save the model to
    vector< vector<float> > training_data; 
    vector<float> labels;
    //extract the feature vectors for each image and throw them in a vector
    while(file.has_next()){ 
	char *fname = file.nextimg(); 
	cout << (file.is_pos()? "Pos" : "Neg") << ":" << fname << endl;
	vector<float> features;
	calculate_features_from_input(fname, features, desc);  
	if(!features.empty()){
	    training_data.push_back(features);
	    labels.push_back(file.is_pos()? 1.0 : -1.0);
	}
    }
    //convert training vector to a mat
    Mat training_mat(training_data.size(), training_data[0].size(), CV_32FC1);
    for(int i=0; i < training_mat.rows; ++i){
	for(int j=0; j < training_mat.cols; ++j){
	    training_mat.at<float>(i, j) = training_data.at(i).at(j);
	}
    }
    //convert the label vector to a mat
    Mat training_label(labels.size(), 1, CV_32FC1);
    flv2mat(labels, training_label);
    
    // Set up SVM's parameters
    CvSVMParams params;
    params.svm_type    = CvSVM::C_SVC;
    params.kernel_type = CvSVM::LINEAR;
    params.term_crit   = cvTermCriteria(CV_TERMCRIT_ITER, iter, epi);
    params.C = c;
    
    LinearSVM *svm = new LinearSVM;
    svm->train(training_mat, training_label, Mat(), Mat(), params);
    training_mat.release();
    training_label.release();
    DsReader dataset(argv[1]);
    if(!dataset.data_exists()){
	cout << "Error: " << argv[1] << " doesn't exist! Cannot test svm." << endl;
    }
    
    struct DsImage img_data;
    //loop over all the images in the data set
    int cur =0;
    int positive_count = 0;
    int negative_count = 0;
    for(int i=0; i < 2; ++i){
	cout << "Running iteration:" << i << endl;
	float ratio  = hn_train(dataset, svm, training_data, labels, type, scale, i+1);
	cout << "fp:tp - " << ratio << endl;
	//retrain the svm with the expanded sample set
	//convert training vector to a mat
	cout << "going from vector to mat" << endl;
	Mat retraining_mat(training_data.size(), training_data[0].size(), CV_32FC1);
	for(int i=0; i < retraining_mat.rows; ++i){
	    for(int j=0; j < retraining_mat.cols; ++j){
		retraining_mat.at<float>(i, j) = training_data.at(i).at(j);
	    }
	}
	//convert the label vector to a mat
	Mat retraining_label(labels.size(), 1, CV_32FC1);
	flv2mat(labels, retraining_label);
	cout << "retraining" << endl;
	//retrain with the false positives
	svm->train(retraining_mat, retraining_label, Mat(), Mat(), params);
    }
    positive_count = 0;
    negative_count = 0;
    int fp = 0;
    int total_p = 0;
    int fn = 0;
    int total_n = 0;
    int total = 0;
    dataset.reset();

    vector<float> support_vector;
    svm->getSupportVector(support_vector);
    gpu::HOGDescriptor gdesc;
    gdesc.setSVMDetector(support_vector);

    while(dataset.has_next()){
	//increment the image counter
	cur++;
	//load the next image info into the img_data struct
	dataset.next(img_data);
	//load the image
	Mat cimg = imread(img_data.path, 1);	

	//if the image is empty(it doesnt exist) skip it
	if(cimg.cols == 0) 
		continue;

	Mat img;
	cvtColor(cimg, img, CV_BGR2GRAY);
	
	//upload the image to the gpu for detection
	gpu::GpuMat gimg;
	gimg.upload(img);
	//create the vector to hold the results
	vector<Rect> matches;
	//perform the multiscale match(on the gpu)
	gdesc.detectMultiScale(gimg, matches, 0, Size(), Size(0, 0), scale, rects);
	for(int i=0; i < matches.size(); ++i){
	    rectangle(cimg, matches[i], Scalar(255, 0, 0), 4);		
	}
	
	//set the default state to the image should be negative
	int ispos = 0;
	for(int n=0; n < img_data.tags.size(); ++n){
	    string cls = dataset.get_class(img_data.tags[n].clss);
	    //first check if this tag is even the right type of object
	    if(cls == type){
		ispos = 1;
	    }
	}
	total++;
	if(matches.size() > 0){
	    //putText(img, string(img_data.path), cvPoint(30,30), 
	    //    FONT_HERSHEY_COMPLEX_SMALL, 0.4, cvScalar(200,200,250), 1, CV_AA);
	    imwrite("/var/www/training_tests/"+to_string(cur)+".jpg", cimg);
	    cout << "writing image: " << "/var/www/training_tests/"+to_string(cur)+".jpg" << endl;
	    if(ispos){
		total_p++;
		positive_count++;
	    }else{
	    	total_n++;
	    	fp++;
	    }
	}else{
	    if(ispos){
	    	total_p++;
		imwrite("/var/www/fn/"+to_string(cur)+".jpg", cimg);
		fn++;
	    }else{
		total_n++;
	    }
	}
	cout << "Testing with image " << cur << "/" << dataset.size() << endl;
    }
    cout << positive_count << " Stop signs recognized ("<< (((float)positive_count)/total_p)*100 << "% of all stopsigns) " << endl; 
    cout << "Total Accuracy: " << (((float)(total_p-fp)+(total_n-fn))/total)*100 <<  "%" << endl;
    cout << "False Pos: " << fp << " (" << ((float)fp/total_n)*100.0 << "% of all non-stopsigns)" << endl;
    cout << "False Neg: " << fn << " (" << ((float)fn/total_p)*100.0 << "% of all stopsigns)" << endl;
    //save the svm to output file
    svm->save(argv[3]);
    cout << argv[3] << " written " << endl;
    exit(0);
}
