#include "trainer.h"

using namespace cv;
using namespace std;
using namespace JSON;

SampleDetector::SampleDetector(CvSVM *psvm, int width, int height){
    svm = psvm;
    tag_width = width;
    tag_height = height;
}

/*
 * Adds the pos (x and y) of a matching vector
 */
void SampleDetector::add_pos(int row, int col, vector<float> &desc){
    struct SdMatch pos = {row, col, desc};
    found.push_back(pos);
}

void SampleDetector::scan_row(Mat &img, HOGDescriptor& hog, int row, int step_size){
    vector<float> vfeatures;
    for(int c=0, max_col=img.cols-tag_width; c < max_col; c+=step_size){ 
        Mat block(img, Rect(c, row, tag_width, tag_height)); 
        Mat showblock(block);
        vfeatures.clear(); 
        int s = calculate_features_from_input(block, vfeatures, hog); 
        if(s != 0){
            Mat features(1, vfeatures.size(), CV_32FC1); 
            flv2mat(vfeatures, features);
            float clss = svm->predict(features);
            if(clss == 1.0){
                add_pos(row, c, vfeatures);           
                imshow("test", showblock);
                waitKey(1);
            }
        }
    }
}

void SampleDetector::clear(){
    found.clear();
}

vector<struct SdMatch> SampleDetector::get_matches(){
    return found;
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
        Array clsses = (Array)root["classes"];
        for(int i=0; i < clsses.size(); ++i){
            classes.push_back((string)clsses[i]);
        }

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
 * Returns the number of images
 */
int DsReader::size(){
    return ((Array)root["images"]).size();
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
    
    imgdata.tags.clear();
    //get the tags and convert them to a struct
    struct DsTag tag;
    Array tags = (Array)element["tags"];
    for(int i=0; i < tags.size(); ++i){
        //use a wrapper struct for the pos(since vector doesn't like arrays)
        struct DsPos temp_pos = {0, 0};
        //cast it to an int so we can access it like an array
        int *int_pos = &temp_pos.x;
        Array pos = (Array)tags[i]["pos"]; 
        for(int j=0; j < pos.size(); ++j){
            int_pos[j] = pos[j].as_int();
        }
        tag.pos = temp_pos;

        tag.scale = tags[i]["scale"].as_float();
        tag.clss = tags[i]["class"].as_int();
    }
    imgdata.tags.push_back(tag);
    current++;
}


int calculate_features_from_input(Mat &image, vector<float>& featureVector,
        HOGDescriptor& hog) 
{
        if(image.empty()){
            featureVector.clear();
            return 0;
        }
        if (image.cols != hog.winSize.width || image.rows != hog.winSize.height) {
            featureVector.clear();
            return 0;
        }
        hog.compute(image, featureVector); 
        image.release();
        return 1;
}


void calculate_features_from_input(char* imageFilename, vector<float>& featureVector,
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

void flv2mat(vector<float> &values, Mat &cp){
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

/*
 * Calculates the overlap of two rectangles
 */
int get_overlap(int x1, int y1, int x2, int y2, int width, int height){
    int xover = max(0, min(x1+width,x2+width) - max(x1,x2));
    int yover = max(0, min(y1+height,y2+height) - max(y1, y2));
    return xover*yover;
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
    string type = "puck";

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
        calculate_features_from_input(fname, features, desc);  
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
    int i =0;
    int positive_count = 0;
    int negative_count = 0;
    while(dataset.has_next()){
        i++;
        dataset.next(img_data);
        Mat img = imread(img_data.path, 0);
        SampleDetector detect(svm, dataset.tag_width(), dataset.tag_height()); 
        for(int i=0, max_rows=img.rows-dataset.tag_height(); i < max_rows; i+=step_size){
            //note: implement this without threads first, but make it in a class so moving to threads is posible
            //spawn a thread that will check every col in the row for matches 
            detect.scan_row(img, desc, i, step_size);
        }
        vector<struct SdMatch> matches = detect.get_matches();
        
        int tag_area = dataset.tag_width()*dataset.tag_height();    
        for(int i=0; i < matches.size(); ++i){ 
            int ispos = false;
            for(int n=0; n < img_data.tags.size(); ++n){
                string cls = dataset.get_class(img_data.tags[n].clss);
                //first check if this tag is even the right type of object
                if(cls != type || cls == "na"){
                    break;
                }
                //get the overlap of the found object and the tag
                int overlap = get_overlap(matches[i].x, matches[i].y,
                                          img_data.tags[n].pos.x, img_data.tags[n].pos.y, 
                                          dataset.tag_width(), dataset.tag_height());

                float ratio = ((float)overlap)/tag_area;
                //cout << ratio << endl;
                if(ratio > .5){
                    ispos = true;
                }
            }
            if(ispos){
                positive_count++; 
            }else{
                negative_count++;
                training_data.push_back(matches[i].desc);
                labels.push_back(-1.0);
            }
        }
        detect.clear();
        cout << "image " << i << "/" << dataset.size() << endl;
    }
    //print the pos and neg counts and percentages
    cout << positive_count << " Positve, " << negative_count << " Negative " << ((float)positive_count/negative_count) << "%" << endl;
    //retrain the svm with the expanded sample set
    //use same loop as before without adjusting the training set
    //print out the pos and neg ratios
    //save the svm to output file

}
