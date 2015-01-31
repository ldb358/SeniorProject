#include "trainer.h"

using namespace cv;
using namespace std;


FileReader::FileReader(char *filename){
    myfile.open(filename);
    img = new char[200];
    char_count = 0;
    line_count = 0;
    image_count = 0;
}

char *FileReader::nextimg(){
    if(char_count == line.length()){
        loadline();
    }
    unsigned int len = line.length(); 
    char c = 0;
    int start = char_count;
    while(c != ' ' && char_count < len){
        c = line[char_count];
        char_count++;
    }
    line.copy(img, char_count-start-1, start); 
    return img;
}

int FileReader::has_next(){
    return !(line_count == 2 && char_count == line.length());
}

int FileReader::is_pos(){
    return line_count==1;
}

void FileReader::loadline(){
    getline (myfile,line);
    line_count++;
    char_count = 0;
}

int FileReader::is_open(){
    return myfile.is_open();
}


void calculateFeaturesFromInput(char* imageFilename, vector<float>& featureVector, HOGDescriptor& hog) 
{
        Mat image;
        image = imread(imageFilename);
        if(image.empty()){
            featureVector.clear();
            cout << "Image " << imageFilename << " Empty, skipping." << endl;
            return;
        }
        waitKey(0);
}


int main(int argc, char**argv){
    if(argc < 2){
        cout << "You need to provide file contain a list of image to use for training" 
            << endl;
        return -1;
    }
    FileReader file(argv[1]);
    if(!file.is_open()){
        cout << "File couldn't be opened... exiting.";
        return 1;
    }
    while(file.has_next()){ 
        char *fname = file.nextimg(); 
        cout << (file.is_pos()? "Pos" : "Neg") << ":" << fname << endl;
        vector<float> features;
        HOGDescriptor desc;
        calculateFeaturesFromInput(fname, features, desc);

    }
    
}
