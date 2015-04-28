#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include<opencv2/core/core.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/gpu/gpu.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/ml/ml.hpp>

#include "includes/LinearSVM.h"

using namespace std;
using namespace cv;

#define MODEL "stopsign.yaml" 


class DetectorServer{
    private:
    	//the svm we use for recognition
	LinearSVM svm;
	gpu::HOGDescriptor desc;
    	int img_count = 0;
    public:
	int processImage(Mat &img);
	DetectorServer();
};
