#include "detector_server.h"


DetectorServer::DetectorServer(){
    //load the model used to classify
    svm.load("/home/sp/SeniorProject/models/stopsign.yaml"); 
    //convert the svm to a vector for floats
    vector<float> support_vector;
    svm.getSupportVector(support_vector);
    //load this into our detector
    desc.setSVMDetector(support_vector);
}


int DetectorServer::processImage(Mat &img){
    if(img.cols == 0) return 0;    
    Mat gray_img;
    img_count++;
    cvtColor(img, gray_img, CV_BGR2GRAY); 
    
    //our large image holder
    gpu::GpuMat gimg;
    //create the vector to hold the results
    vector<Rect> matches;
    
    //upload our image to the gpu
    gimg.upload(gray_img);
    desc.detectMultiScale(gimg, matches, 0, Size(), Size(0, 0), 1.04, 5);
    for(int i=0; i < matches.size(); ++i){
	rectangle(gray_img, matches[i], Scalar(255, 0, 0));		
    }    
    imwrite("/var/www/training_tests/test"+string(img_count)+".jpg", gray_img);

    return matches.size()>0;
}
