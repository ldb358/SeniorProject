#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include<opencv2/core/core.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/gpu/gpu.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/ml/ml.hpp>
#include "includes/FileReader.h"

using namespace std;
using namespace cv;

struct matches{
    int count;
    vector< int[2] > boxes; //a vector of all of the matches where [0] = row  and [1] = col
};

class SampleDetector{
    private:
        CvSVM *svm;

    public:
        void add_match(int row, int col);
        struct matches *scan_row(Mat &img, int row, int step_size);
};

