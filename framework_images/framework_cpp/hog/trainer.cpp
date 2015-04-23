#include "trainer.h"
#include<unistd.h>

using namespace cv;
using namespace std;

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
    SampleDetector detect(svm, dataset.tag_width(), dataset.tag_height());
    //set up our detection threads
    pthread_t threads[NUM_THREADS];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    for(int i=0; i < NUM_THREADS; ++i){
	int rc = pthread_create(&threads[i], &attr, SampleDetector::start_thread, (void*)&detect);
	if(rc){
	    cout << "Pthread failed to create threads with error code:" << rc << endl;
	    return 0;
	}
    }
    pthread_attr_destroy(&attr);
    while(dataset.has_next()){
	cur++;
	dataset.next(img_data);
	Mat img = imread(img_data.path, 0);
	detect.set_img(img);
	for(int i=0, max_rows=img.rows-dataset.tag_height(); i < max_rows; i+=step_size){
	    detect.scan_row_threaded(desc, i, step_size);
	}
	while(detect.tworking()){}
	vector<struct SdMatch> matches = detect.get_matches();
	
	int tag_area = dataset.tag_width()*dataset.tag_height();    
	int pos_img = 0;
	for(int i=0; i < matches.size(); ++i){ 
	    int ispos = false;
	    for(int n=0; n < img_data.tags.size(); ++n){
		string cls = dataset.get_class(img_data.tags[n].clss);
		//first check if this tag is even the right type of object
		if(cls != type || cls == "na"){
		    break;
		}
		//get the overlap of the found object and the tag
		int overlap = get_overlap(matches[i].x, matches[i].y,
					  img_data.tags[n].pos.x, img_data.tags[n].pos.y, 
					  dataset.tag_width(), dataset.tag_height(),
					  img_data.tags[n].scale);
		
		float ratio = ((float)overlap)/tag_area; 
		if(ratio > overlap_thresh){
		    ispos = true;
		    //perform non-max supression and add as positive samples
		    pos_img = 1;
		}
	    }
	    if(ispos){
		positive_count++;  
	    }else{
		negative_count++;
		training_data.push_back(matches[i].desc);
		labels.push_back(-1.0);
	    }
	}
	img.release();	
	detect.clear();
	//divides the dataset into two parts, training and testing
	if(cur > 70) break;
	cout << "image " << cur << "/" << dataset.size() << " Status:" << pos_img <<  endl;
    }
    //print the pos and neg counts and percentages
    cout << positive_count << " Positve, " << negative_count << " Negative " << ((float)positive_count/negative_count) << "+/-" << endl;
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

    positive_count = 0;
    negative_count = 0;
    int fp = 0;
    int total_p = 0;
    int fn = 0;
    int total_n = 0;
    int total = 0;
    //dataset.reset();

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
	Mat img = imread(img_data.path, 0);	
	//if the image is empty(it doesnt exist) skip it
	if(img.cols == 0) 
		continue;
	//upload the image to the gpu for detection
	gpu::GpuMat gimg;
	gimg.upload(img);
	//create the vector to hold the results
	vector<Rect> matches;
	//perform the multiscale match(on the gpu)
	gdesc.detectMultiScale(gimg, matches, 0, Size(), Size(0, 0), scale, rects);
	for(int i=0; i < matches.size(); ++i){
	    rectangle(img, matches[i], Scalar(255, 0, 0));		
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
	    putText(img, string(img_data.path), cvPoint(30,30), 
	        FONT_HERSHEY_COMPLEX_SMALL, 0.4, cvScalar(200,200,250), 1, CV_AA);
	    imwrite("/var/www/training_tests/"+to_string(cur)+".jpg", img);
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
		imwrite("/var/www/fn/"+to_string(cur)+".jpg", img);
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
    while(detect.cur_threads() > 0){
	detect.kill_thread();
    }
    for(int i=0; i < NUM_THREADS; ++i){
	pthread_join(threads[i], (void**)NULL);
    } 
    exit(0);
}
