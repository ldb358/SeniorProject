#include "trainer.h"

using namespace cv;
using namespace std;

/*
 * DsReader: Reads a json data set file created by tagger
 *
 *
 */

void calculateFeaturesFromInput(char* imageFilename, vector<float>& featureVector,
        HOGDescriptor& hog) 
{
        Mat image;
        image = imread(imageFilename);
        if(image.empty()){
            featureVector.clear();
            cout << "Image " << imageFilename << " Empty, skipping." << endl;
            return;
        }
        if (image.cols != hog.winSize.width || image.rows != hog.winSize.height) {
            featureVector.clear();
            cout << imageFilename << " Couldn't use for training since the dimensions are wrong" << endl;
            return;
        }
        hog.compute(image, featureVector); 
        image.release();
}

void flv2mat(vector<float> values, Mat &cp){
    int orient  = (cp.rows > cp.cols);
    for(int i=0; i < cp.rows; ++i){
        for(int j=0; j < cp.cols; ++j){
            if(orient){
                cp.at<float>(i, j) = values.at(i);
            }else{
                cp.at<float>(i, j) = values.at(j);
            }
        }
    }
}


int main(int argc, char**argv){
    if(argc < 4){
        cout << "usage: [json file for tags] [file with list of images for training ] [file to save model to]"
            << endl;
        return -1;
    }
    FileReader file(argv[2]);
    if(!file.is_open()){
        cout << "File couldn't be opened... exiting.";
        return 1;
    }
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
        calculateFeaturesFromInput(fname, features, desc);  
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
    params.term_crit   = cvTermCriteria(CV_TERMCRIT_ITER, 100, 1e-6);
    
    CvSVM *svm = new CvSVM;
    svm->train(training_mat, training_label, Mat(), Mat(), params);
    
    //loop over all the images in the data set
        //rows = (image_height - box_height)/step_size
        //cols = (image_width - box_width)/step_size
        //foreach row
            //note: implement this without threads first, but make it in a class so moving to threads is posible
            //spawn a thread that will check every col in the row for matches 
            //if there is a match it will create a new match class to a vector
        //foreach match
            //if the match is valid:
                //++ the positive matches
            //else
                //++ the negitive matches
                //add the negative image to the training set
    //print the pos and neg counts and percentages
    //retrain the svm with the expanded sample set
    //use same loop as before without adjusting the training set
    //print out the pos and neg ratios
    //save the svm to output file

}
