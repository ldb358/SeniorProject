// Combined Algorithm 2 and 5: Color Detection and Shape Detection
//#include "algorithm.h"
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>

#define WIDTH 640
#define HEIGHT 480
#define COLOR_DETECT 1
#define SHAPE_DETECT 1
#define MINRED .05

#ifndef RENDER
//#define RENDER
#endif

using namespace cv;
using namespace std;

int processImage(Mat &img);
int shapeDetect(Mat img);
Mat colorDetect(Mat imgHSV, Scalar low, Scalar high);
Scalar getFrac(Mat imgThresholded);

int processImage(Mat Img) {
	Size size(WIDTH,HEIGHT);
	resize(Img, Img, size);
	Mat imgHSV;
	int sign = 0;

	if (SHAPE_DETECT) {
		// shape processing
		Mat canny_output;
		Mat Img_gray;
		int thresh = 120;
		cvtColor(Img, Img_gray, CV_BGR2GRAY);
		GaussianBlur(Img_gray, Img_gray, Size(3,3), 0);
		Canny(Img_gray, canny_output, thresh, thresh*2, 3);
#ifdef RENDER
		namedWindow("Canny_before", CV_WINDOW_AUTOSIZE);
		imshow("Canny_before", canny_output);
#endif
		sign = shapeDetect(canny_output);
		if (sign) 
			return sign;
	}

	if (COLOR_DETECT) {
		// set range for detection of red
		int iLowH = 160;
        	int iHighH = 179;
	
	    	int iLowS = 0; 
	    	int iHighS = 255;
	
	    	int iLowV = 0;
	    	int iHighV = 255;

		//Convert the captured frame from BGR to HSV
		cvtColor(Img, imgHSV, COLOR_BGR2HSV);

		Mat imgThresholded = colorDetect(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV));
		
		// get proportion of red in image
		Scalar frac = getFrac(imgThresholded);
		if (frac[0] >= MINRED) {
			sign = 1;
		} 
		else {
			sign = 0;
		}
	}
	
	return sign;
}

int shapeDetect(Mat img) {
	vector<vector<Point> > contour;
	vector<vector<Point> > result;
	vector<Vec4i> hierarchy;
	int epsilon = 16;
	int sign = 0;

	//finding all contours in the image
	findContours(img, contour, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, Point(0,0));

	//Draw contours
	result.resize(contour.size());
	Mat drawing = Mat::zeros(img.size(), CV_8UC3);
	Scalar color = Scalar(255, 0, 0);
	for (int i = 0; i < contour.size(); i++) {
		approxPolyDP(contour[i], result[i], epsilon, true);
		if (result[i].size() == 8 && contourArea(result[i]) > 2500)
		{
			sign = 1;
#ifdef RENDER
			drawContours(drawing, result, i, color, 2, 8, hierarchy, 0, Point());
			continue;
#endif
			break;
		}
	}

#ifdef RENDER
	namedWindow("Contours", CV_WINDOW_AUTOSIZE);
	imshow("Contours", drawing);
#endif

	return sign;
}

Mat colorDetect(Mat imgHSV, Scalar lowV, Scalar highV) {
	Mat imgThresholded;

	// Threshold the image
	inRange(imgHSV, lowV, highV, imgThresholded); 
      
	// morphological opening (remove small objects from the foreground)
	erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
	dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) ); 

	// morphological closing (fill small holes in the foreground)
	dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) ); 
	erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

	return imgThresholded;
}

Scalar getFrac(Mat imgThresholded) {
	Scalar psum = sum(imgThresholded) / 255.0;
	Scalar frac = (psum) / (1.0 * WIDTH * HEIGHT);
	cout << "Percentage Red: " << frac[0] << endl;
	return frac;
}
