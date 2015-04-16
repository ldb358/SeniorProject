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
#include "includes/SampleDetector.h"
#include "includes/LinearSVM.h"
#include "../../../dataset_reader/DsReader.h"
#include "includes/helper.h"
using namespace std;
using namespace cv;

#define NUM_THREADS 4
#define STEP_SIZE 10
#define CLASS "stopsign"
//#define CLASS "puck"
