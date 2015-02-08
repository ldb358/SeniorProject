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
#include "includes/json++/json.hh"

using namespace std;
using namespace cv;
using namespace JSON;

struct SdPos{
    int x;
    int y;
};

class SampleDetector{
    private:
        CvSVM *svm;
        vector<struct SdPos> found;
    public:
        void add_pos(int row, int col);
        void scan_row(Mat &img, int row, int step_size);
        vector<struct SdPos> get_matches();
};


struct DsImage{
    char *path;
    vector<struct DsTag> tags;
};

struct DsTag{
    vector<struct DsPos> pos;
    double scale;
    int clss;
};

struct DsPos{
    int x;
    int y;
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
        int data_exists();
        void next(struct DsImage &imgdata);
        int tag_width(){ return twidth; }
        int tag_height(){ return theight; }
};
