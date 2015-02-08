#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include<opencv2/core/core.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/gpu/gpu.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/ml/ml.hpp>
#include<math.h>
#include "includes/FileReader.h"
#include "includes/json++/json.hh"

using namespace std;
using namespace cv;
using namespace JSON;

struct SdMatch{
    int x;
    int y;
    vector<float> desc;
};

struct SdPos{
    int x;
    int y;
};

class SampleDetector{
    private:
        CvSVM *svm;
        vector<struct SdMatch> found;
        int tag_width;
        int tag_height;
    public:
        SampleDetector(CvSVM *psvm, int width, int height);
        void add_pos(int row, int col, vector<float> &desc);
        void scan_row(Mat &img, HOGDescriptor& hog, int row, int step_size);
        void clear();
        vector<struct SdMatch> get_matches();
};


struct DsImage{
    char *path;
    vector<struct DsTag> tags;
};

struct DsPos{
    int x;
    int y;
};

struct DsTag{
    struct DsPos pos;
    double scale;
    int clss;
};




class DsReader{
    private:
        ifstream jsonfile;
        int current;
        Value root;
        string data_path;
        int twidth;
        int theight; 
        char *path_buffer;
        int status;
        vector<string> classes;
    public:
        DsReader(char* filename);
        int has_next();
        int size();
        int data_exists();
        void next(struct DsImage &imgdata);
        int tag_width(){ return twidth; }
        int tag_height(){ return theight; }
        string get_class(int i){ return i<classes.size()?classes[i] : "na"; }
};

void calculate_features_from_input(char* imageFilename, vector<float>& featureVector,
        HOGDescriptor& hog); 

int calculate_features_from_input(Mat &image, vector<float>& featureVector,
        HOGDescriptor& hog); 

void flv2mat(vector<float> &values, Mat &cp);

int get_overlap(int x1, int y1, int x2, int y2, int width, int height);

