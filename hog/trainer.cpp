#include "trainer.h"

using namespace cv;
using namespace std;
using namespace JSON;

/*
 * Adds the pos (x and y) of a matching vector
 */
void SampleDetector::add_pos(int row, int col){
    struct SdPos pos = { row, col };
    found.push_back(pos);
}

void SampleDetector::scan_row(Mat &img, int row, int step_size){
    for(int c=0; c < img.cols; c+=step_size){

    }
}


/*
 * DsReader: Reads a json data set file created by tagger.py
 */
DsReader::DsReader(char *filename){
    status = 0;
    try{
        root = parse_file(filename);
        status = 1;   
    }catch(std::runtime_error e){
        status = 0;
    }
    if(status != 0){
        twidth = root["width"].as_int();
        theight = root["height"].as_int();
    }
    path_buffer = new char[400];
    current = 0;
}

/*
 * Returns the next image tag object
 */
int DsReader::has_next(){
    return ((Array)root["images"]).size() > current;
}

/*
 * can be used to check if there was any data in the file
 */
int DsReader::data_exists(){
    return status;
}

/*
 * takes a struct by reference and populates it for the next image then increments
 * the counter
 */
void DsReader::next(struct DsImage &imgdata){
    Object element = root["images"][current];
    //get the path
    string path = (string)element["name"];
    imgdata.path = path_buffer;
    strcpy(imgdata.path, path.c_str());
    
    //get the tags and convert them to a struct
    struct DsTag tag;
    Array tags = (Array)element["tags"];
    for(int i=0; i < tags.size(); ++i){
        //use a wrapper struct for the pos(since vector doesn't like arrays)
        struct DsPos temp_pos = { 0, 0};
        //cast it to an int so we can access it like an array
        int *int_pos = &temp_pos.x;
        Array pos = (Array)tags[i]["pos"]; 
        for(int j=0; j < pos.size(); ++j){
            int_pos[j] = pos[j].as_int();
        }
        tag.pos.push_back(temp_pos);

        tag.scale = tags[i]["scale"].as_float();
        tag.clss = tags[i]["class"].as_int();
    }
    current++;
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
            cout << imageFilename << " Couldn't use for training since the dimensions are wrong" << endl;
            return;
        }
        hog.compute(image, featureVector); 
        image.release();
}

void flv2mat(vector<float> values, Mat &cp){
    int orient  = (cp.rows > cp.cols);
    for(int i=0; i < cp.rows; ++i){
        for(int j=0; j < cp.cols; ++j){
            if(orient){
                cp.at<float>(i, j) = values.at(i);
            }else{
                cp.at<float>(i, j) = values.at(j);
            }
        }
    }
}


int main(int argc, char**argv){
    if(argc < 4){
        cout << "usage: [json file for tags] [file with list of images for training ] [file to save model to]"
            << endl;
        return -1;
    }
    FileReader file(argv[2]);
    if(!file.is_open()){
        cout << "File couldn't be opened... exiting.";
        return 1;
    }

    //arguments
    int step_size = 10;


    // save our descriptors in the object(since we are only calculating one at a time
    HOGDescriptor desc;
    //output file to save the model to
    vector< vector<float> > training_data; 
    vector<float> labels;
    //extract the feature vectors for each image and throw them in a vector
    while(file.has_next()){ 
        char *fname = file.nextimg(); 
        cout << (file.is_pos()? "Pos" : "Neg") << ":" << fname << endl;
        vector<float> features;
        calculateFeaturesFromInput(fname, features, desc);  
        if(!features.empty()){
            training_data.push_back(features);
            labels.push_back(file.is_pos()? 1.0 : -1.0);
        }
    }
    //convert training vector to a mat
    Mat training_mat(training_data.size(), training_data[0].size(), CV_32FC1);
    for(int i=0; i < training_mat.rows; ++i){
        for(int j=0; j < training_mat.cols; ++j){
            training_mat.at<float>(i, j) = training_data.at(i).at(j);
        }
    }
    //convert the label vector to a mat
    Mat training_label(labels.size(), 1, CV_32FC1);
    flv2mat(labels, training_label);
    
    // Set up SVM's parameters
    CvSVMParams params;
    params.svm_type    = CvSVM::C_SVC;
    params.kernel_type = CvSVM::LINEAR;
    params.term_crit   = cvTermCriteria(CV_TERMCRIT_ITER, 100, 1e-6);
    
    CvSVM *svm = new CvSVM;
    svm->train(training_mat, training_label, Mat(), Mat(), params);
    DsReader dataset(argv[1]);
    if(!dataset.data_exists()){
        cout << "Error: " << argv[1] << " doesn't exist! Cannot test svm." << endl;
    }

    struct DsImage img_data;
    //loop over all the images in the data set
    while(dataset.has_next()){
        dataset.next(img_data);
        Mat img = imread(img_data.path, 0);
        //imshow("test", img);
        //waitKey(0);
        int rows = (img.rows - dataset.tag_height())/step_size;
        int cols = (img.cols - dataset.tag_width())/step_size;
        for(int i=0; i < rows; ++i){
            //note: implement this without threads first, but make it in a class so moving to threads is posible
            //spawn a thread that will check every col in the row for matches 
            //if there is a match it will create a new match class to a vector
        }
        //foreach match
            //if the match is valid:
                //++ the positive matches
            //else
                //++ the negitive matches
                //add the negative image to the training set
    }
    //print the pos and neg counts and percentages
    //retrain the svm with the expanded sample set
    //use same loop as before without adjusting the training set
    //print out the pos and neg ratios
    //save the svm to output file

}
