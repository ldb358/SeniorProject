#include "detector.h"


int processImage(Mat &img){
    if(img.cols == 0) return 0;
    //the cv class used to extract the hog descriptors
    gpu::HOGDescriptor desc;

    //the svm we are going to use
    LinearSVM svm;
    //load the model used to classify
    svm.load("/home/sp/SeniorProject/models/stopsign.yaml"); 

    vector<float> support_vector;
    svm.getSupportVector(support_vector);
    desc.setSVMDetector(support_vector);

    //upload the image to the gpu for detection
    Mat gray_img;
    cvtColor(img, gray_img, CV_BGR2GRAY);
    gpu::GpuMat gimg;
    gimg.upload(gray_img);
    //create the vector to hold the results
    vector<Rect> matches;
    //perform the multiscale match(on the gpu)
    desc.detectMultiScale(gimg, matches, 0, Size(), Size(0, 0), 1.04, 2);

    return (matches.size()>0);
}
