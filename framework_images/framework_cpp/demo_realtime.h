#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <iostream>
using namespace cv;
using namespace std;

#define CLASSIFICATION_ONLY 1
//#define DECTECT_ONLY 1


int processImage(Mat &img); 
int processImage(Mat &img, vector<Rect> &rects);
