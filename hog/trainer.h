using namespace std;
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include<opencv2/core/core.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/gpu/gpu.hpp>
#include<opencv2/highgui/highgui.hpp>

class FileReader{
    private:
        int line_count;
        string line;
        int char_count;
        int image_count;
        char *img;
        ifstream myfile;
        char *directory;
        void loadline();
    public:
        FileReader(char *filename);
        char *nextimg();
        int is_open();
        int has_next();
        int is_pos();
};
