/*
* Framework for testing image processing algorithms with simple C++ interface
* Last Modified: 1/11/15
*/
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include "algorithm.h"	// interface for image processing algorithms

//#define RENDER		// define if you want to display image

using namespace cv;
using namespace std;

/*
int processImage(Mat Img) {
	int sign = 0;

	// do some processing
	return sign;
}*/

int main(int argc, char *argv[]) {
	if (argc < 2) {
		cout << "Usage: framework image_file" << endl;
		exit(EXIT_FAILURE);
	}
	int render = 0;
	int sign = 0;
#ifndef RENDER
    if(argc == 3){
        render = 1;
    }
#else
    render = 1;
#endif
	// load image data  
	Mat img = imread(argv[1], CV_LOAD_IMAGE_COLOR);

	// perform sign recognition
	sign = processImage(img);

	// display image
    if(render){
        imshow("opencvtest", img);
        // show results of sign recognition algorithm
        sign ? cout << "This is a stop sign.\n" << endl : cout << "This is not a stop sign.\n" << endl;
        waitKey(0);
    }
	return sign;
}

