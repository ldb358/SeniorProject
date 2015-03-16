#include "detect.h"

using namespace cv;
using namespace std;

Ptr<FeatureDetector> detector = new SurfFeatureDetector(20);
Ptr<DescriptorExtractor> desc_extr = DescriptorExtractor::create("SURF");


int main(int argc, char** argv){
	if(argc < 2){
        cout << "usage: [json file for tags] [file to save model to]"
            << endl;
        return -1;
    }
	
	cv::initModule_nonfree();

	//keypoint matcher
	Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("FlannBased");

	//keypoint extracter
	BOWImgDescriptorExtractor extractor(desc_extr, matcher);

	int cluster_count = 1000;
	//read our clustered data from a file
	Mat vocab;
    FileStorage fs(argv[3], FileStorage::READ);
    fs["vocab"] >> vocab;
    fs.release();

	extractor.setVocabulary(vocab);
	
	CvSVM svm;
    svm.load(argv[2]);	
    
    Mat img = imread(argv[1], 0);
	Mat descriptors;
    vector<KeyPoint> keypoints;

    //detect the surf keypoints
    detector->detect(img, keypoints);
    
    //extract the relevent keypoint clusters(usually k-means)
    extractor.compute(img, keypoints, descriptors);
    //get the svm prediction
    int result = svm.predict(descriptors);
    return result;
}
