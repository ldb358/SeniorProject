#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include "json++/json.hh"
#include <vector>
using namespace std;
using namespace JSON;


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
        void reset();
        string get_class(int i){ return i<classes.size()?classes[i] : "na"; }
};

