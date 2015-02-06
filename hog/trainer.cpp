#include "trainer.h"

using namespace cv;
using namespace std;


FileReader::FileReader(char *filename){
    myfile.open(filename);
    img = new char[400];
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
    //zero the image path to prevent wierd left over chars
    bzero(img, 400);
    line[0] = '\0';
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


void calculateFeaturesFromInput(char* imageFilename, vector<float>& featureVector,
        HOGDescriptor& hog) 
{
        Mat image;
        image = imread(imageFilename);
        if(image.empty()){
            featureVector.clear();
            cout << "Image " << imageFilename << " Empty, skipping." << endl;
            return;
        }
        if (image.cols != hog.winSize.width || image.rows != hog.winSize.height) {
            featureVector.clear();
            cout << imageFilename << " Couldn't use for trainging since the dimensions are wrong" << endl;
            return;
        }
        hog.compute(image, featureVector); 
        image.release();
}


int main(int argc, char**argv){
    if(argc < 3){
        cout << "You need to provide file contain a list of image to use for training, and the name of the file to use as a model" 
            << endl;
        return -1;
    }
    FileReader file(argv[1]);
    if(!file.is_open()){
        cout << "File couldn't be opened... exiting.";
        return 1;
    }
    // save our descriptors in the object(since we are only calculating one at a time
    HOGDescriptor desc;
    string tmpf = "model.temp";
    //output file to save the model to
    fstream outfile;
    outfile.open(tmpf.c_str(), ios::out);
    //instructions for training
    outfile << "# Use this file to train, e.g. SVMlight by issuing $ svm_learn -i 1 -a weights.txt " << argv[2] << endl;
    while(file.has_next()){ 
        char *fname = file.nextimg(); 
        cout << (file.is_pos()? "Pos" : "Neg") << ":" << fname << endl;
        vector<float> features;
        calculateFeaturesFromInput(fname, features, desc);  
        if(!features.empty()){
            //output +1(pos) or -1(neg) to class file
            outfile << (file.is_pos()? "+1" : "-1");
            for(unsigned int feature = 0; feature < features.size(); ++feature){
                outfile << " " << feature+1 << ":" << features.at(feature);
            }
            outfile << endl;
        }
    }
    
}
