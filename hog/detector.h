#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include<opencv2/core/core.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/gpu/gpu.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/ml/ml.hpp>
#include "includes/SampleDetector.h"
#include "includes/helper.h"
using namespace std;
using namespace cv;

#define NUM_THREADS 4
#define STEP_SIZE 8
#define WINDOW_WIDTH 64
#define WINDOW_HEIGHT 128
#define MODEL "/home/brenemal/spws/hog/stopsign.yaml" 

#ifndef procimg
#define procimg
int processImage(Mat img);
#endif
