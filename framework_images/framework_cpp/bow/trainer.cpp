#include "trainer.h"

using namespace cv;
using namespace std;

Ptr<FeatureDetector> detector = new SurfFeatureDetector(20);
Ptr<DescriptorExtractor> desc_extr = DescriptorExtractor::create("SURF");

void computeFeaturesWithBOW(DsReader &dataset, Mat &training_data, Mat &labels, 
		BOWImgDescriptorExtractor &extractor){
	
	//reset the dataset
	dataset.reset();
	//our keypoints
	vector<KeyPoint> keypoints;
	
	Mat features;
	Mat img;
	Mat subimg;
	DsImage img_data;
	
	//some useful values for our calculation
	int img_count = 0;
	int block_width = dataset.tag_width();
	int block_height = dataset.tag_height();
	while(dataset.has_next()){
		dataset.next(img_data);
		img = imread(img_data.path, 0);
		cout << "Starting image " << ++img_count << ": ";
		for(int i=0; i < img_data.tags.size(); ++i){	
			//if the image size == max image the box is valid, its just 1 px to big
			int mod =0;
			if(img_data.tags[i].pos.x + block_width == img.rows){
				mod = -1;
			}
			if(img_data.tags[i].pos.x + block_width > img.cols ||
				img_data.tags[i].pos.y + block_height > img.rows){
				cout << "Skipped.";
				continue;
			}	
			subimg = img(Rect(img_data.tags[i].pos.x, img_data.tags[i].pos.y, 
						block_width+mod, block_height));	
			
			//detect the surf keypoints
			detector->detect(subimg, keypoints);
			cout << "detected"; 	
			//extract BOW descriptos
			extractor.compute(img, keypoints, features);
			cout << " -> Extracted OK";	
			string cls = dataset.get_class(img_data.tags[i].clss);
			//set the label based on the class
			if(features.rows == 0) continue;
			labels.push_back((float)((cls != CLASS)? 0 : 1));
			training_data.push_back(features);
		}
		cout << endl;	
	}
}


int main(int argc, char** argv){
	if(argc < 3){
        cout << "usage: [json file for tags] [file to save model to] [file to save vocab to]"
            << endl;
        return -1;
    }
	
	cv::initModule_nonfree();

	//keypoint vector
	vector<KeyPoint> keypoints;

	//keypoint matcher
	Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("FlannBased");

	vector< DMatch > matches;

	//keypoint extracter
	BOWImgDescriptorExtractor extractor(desc_extr, matcher);


	//mat to save our extracted descriptors in
	Mat training_descriptors(1,extractor.descriptorSize(),extractor.descriptorType());


	Mat img;
	Mat descriptors;
	Mat subimg;
	cout << "Step 1: Building Vocab" << endl;
	
	DsReader dataset(argv[1]);
    if(!dataset.data_exists()){
        cout << "Error: " << argv[1] << " doesn't exist! Cannot test svm." << endl;
    }
	
	struct DsImage img_data;
	int block_width  = dataset.tag_width();
	int block_height = dataset.tag_height();
	int img_count = 0;
	//loop over every image in the dataset
	while(dataset.has_next()){
		dataset.next(img_data);
		img = imread(img_data.path, 0);
		cout << "Starting image " << ++img_count << ": ";
		for(int i=0; i < img_data.tags.size(); ++i){
			string cls = dataset.get_class(img_data.tags[i].clss);
			//we only want tags of our current training class
			if(cls != CLASS){
				//cout << "Not Right Class -> Skipping";
				//continue;
			}
			//if the image size == max image the box is valid, its just 1 px to big
			int mod =0;
			if(img_data.tags[i].pos.x + block_width == img.rows){
				mod = -1;
			}
			if(img_data.tags[i].pos.x + block_width > img.cols ||
				img_data.tags[i].pos.y + block_height > img.rows){
				cout << "Skipped.";
				continue;
			}
			subimg = img(Rect(img_data.tags[i].pos.x, img_data.tags[i].pos.y, 
						block_width+mod, block_height));	
			
			//detect the surf keypoints
			detector->detect(subimg, keypoints);
			cout << "detected"; 	
			//extract the relevent keypoint clusters(usually k-means)
			desc_extr->compute(img, keypoints, descriptors);
			cout << " -> Extracted OK";	
			training_descriptors.push_back(descriptors);
		}
		cout << endl;	
	}
	cout << "Training Vocab" << endl;
	cout << training_descriptors.size() << endl;
	int cluster_count = 1500;
	//create a trainer with 1000 clusters
	BOWKMeansTrainer bow_trainer(cluster_count);

	//add our training data
	bow_trainer.add(training_descriptors);
	//run the clustering
	Mat vocab = bow_trainer.cluster();
	FileStorage fs(argv[3], FileStorage::WRITE);
	fs << "vocab" << vocab;
	fs.release();

	cout << "Step 2: train the svm" << endl;
	
	Mat labels(0, 1, CV_32FC1);
	Mat training_data(0, cluster_count, CV_32FC1);

	extractor.setVocabulary(vocab);
	computeFeaturesWithBOW(dataset, training_data, labels, extractor);
	
	double c=16;//16
	double gamma=1.1;
	CvSVMParams params;
	params.kernel_type=CvSVM::RBF;
	params.svm_type=CvSVM::C_SVC;
	params.gamma=gamma;
	params.C=c;
	params.term_crit=cvTermCriteria(CV_TERMCRIT_ITER,100,0.000001);

	CvSVM svm;
	cout << "Training the Machine." << endl;
	svm.train(training_data,labels,cv::Mat(),cv::Mat(),params);
	svm.save(argv[2]);	
	cout << "Step 3: test our results" << endl;
	
	dataset.reset();
	img_count = 0;
	int correct = 0;
	int fp = 0;
	int tp = 0;
	int fn = 0;
	int tn = 0;
	while(dataset.has_next()){
		dataset.next(img_data);
		img = imread(img_data.path, 0);
		cout << "Starting image " << ++img_count << ": ";
		for(int i=0; i < img_data.tags.size(); ++i){
			string cls = dataset.get_class(img_data.tags[i].clss);
			//if the image size == max image the box is valid, its just 1 px to big
			int mod =0;
			//detect the surf keypoints
			detector->detect(img, keypoints);
			//extract the relevent keypoint clusters(usually k-means)
			extractor.compute(img, keypoints, descriptors);
			int result = svm.predict(descriptors);
			int expected = (cls == CLASS);
			cout << "Expected:" << expected << " Got:" << result;
			if(expected){
				tp++;
			}else{
				tn++;
			}
			if(result == expected){
				correct++;
			}else if(result == 1){
				fp++;
			}else{
				fn++;
			}
		}
		cout << endl;	
	}
	cout << "C: " << c << " Gamma: " << gamma << " "; 
	cout << "Total tests:" << img_count << " Correct:" << correct << "(" << 
				((float)correct/img_count)*100.0 << "%) False Pos:" <<
				fp << "(" << ((float)fp/tn)*100.0 << "%) False Neg:" <<
				fn << "(" << ((float)fn/tp)*100.0 << "%)" << endl; 
}
