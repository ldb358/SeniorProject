#include "trainer.h"

using namespace cv;
using namespace std;

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
    int step_size = STEP_SIZE;
    string type = CLASS;

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
    params.term_crit   = cvTermCriteria(CV_TERMCRIT_ITER, 250, 1e-10);
    params.C = 200;
    
    CvSVM *svm = new CvSVM;
    svm->train(training_mat, training_label, Mat(), Mat(), params);
    DsReader dataset(argv[1]);
    if(!dataset.data_exists()){
        cout << "Error: " << argv[1] << " doesn't exist! Cannot test svm." << endl;
    }

    struct DsImage img_data;
    //loop over all the images in the data set
    int cur =0;
    int positive_count = 0;
    int negative_count = 0;
    SampleDetector detect(svm, dataset.tag_width(), dataset.tag_height());
    //set up our detection threads
    pthread_t threads[NUM_THREADS];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    for(int i=0; i < NUM_THREADS; ++i){
        int rc = pthread_create(&threads[i], &attr, SampleDetector::start_thread, (void*)&detect);
        if(rc){
            cout << "Pthread failed to create threads with error code:" << rc << endl;
            return 0;
        }
    }
    pthread_attr_destroy(&attr);
    while(dataset.has_next()){
        cur++;
        dataset.next(img_data);
        Mat img = imread(img_data.path);
        detect.set_img(img);
        for(int i=0, max_rows=img.rows-dataset.tag_height(); i < max_rows; i+=step_size){
            detect.scan_row_threaded(desc, i, step_size);
        }
        while(detect.tworking()){}
        vector<struct SdMatch> matches = detect.get_matches();
        
        int tag_area = dataset.tag_width()*dataset.tag_height();    
        int pos_img = 0;
        for(int i=0; i < matches.size(); ++i){ 
            int ispos = false;
            for(int n=0; n < img_data.tags.size(); ++n){
                string cls = dataset.get_class(img_data.tags[n].clss);
                //first check if this tag is even the right type of object
                if(cls != type || cls == "na"){
                    break;
                }
                cout << Point(matches[i].x, matches[i].y) << Point(matches[i].x+dataset.tag_width(), matches[i].y+dataset.tag_height()) << endl;
                //get the overlap of the found object and the tag
                int overlap = get_overlap(matches[i].x, matches[i].y,
                                          img_data.tags[n].pos.x, img_data.tags[n].pos.y, 
                                          dataset.tag_width(), dataset.tag_height(),
                                          img_data.tags[n].scale);
                
                float ratio = ((float)overlap)/tag_area;
                if(ratio > .8){
                    rectangle(img, Point(matches[i].x, matches[i].y), 
                               Point(matches[i].x+dataset.tag_width(), matches[i].y+dataset.tag_height()), 
                               Scalar(0, 0, 255),
                               2, 8, 0); 
                    //perform non-max supression and add as positive samples
                    ispos = true;
                    pos_img = 1;
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
        cout << "image " << cur << "/" << dataset.size() << " Status:" << pos_img <<  endl;
        if(pos_img){
            imshow("test", img);
            waitKey(0);
        }
    }
    //print the pos and neg counts and percentages
    cout << positive_count << " Positve, " << negative_count << " Negative " << ((float)positive_count/negative_count) << "+/-" << endl;
    //retrain the svm with the expanded sample set
    //convert training vector to a mat
    training_mat.release();
    training_label.release();
    cout << "going from vector to mat" << endl;
    Mat retraining_mat(training_data.size(), training_data[0].size(), CV_32FC1);
    for(int i=0; i < retraining_mat.rows; ++i){
        for(int j=0; j < retraining_mat.cols; ++j){
            retraining_mat.at<float>(i, j) = training_data.at(i).at(j);
        }
    }
    //convert the label vector to a mat
    Mat retraining_label(labels.size(), 1, CV_32FC1);
    flv2mat(labels, retraining_label);
    cout << "retraining" << endl;
    //retrain with the false positives
    svm->train(retraining_mat, retraining_label, Mat(), Mat(), params);

    cur =0;
    positive_count = 0;
    negative_count = 0;
    dataset.reset();
    while(dataset.has_next()){
        cur++;
        dataset.next(img_data);
        Mat img = imread(img_data.path, 0); 
        detect.set_img(img);
        for(int i=0, max_rows=img.rows-dataset.tag_height(); i < max_rows; i+=step_size){
            //note: implement this without threads first, but make it in a class so moving to threads is posible
            //spawn a thread that will check every col in the row for matches 
            detect.scan_row_threaded(desc, i, step_size);
        }
        while(detect.tworking()){}
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
                                          dataset.tag_width(), dataset.tag_height(),
                                          img_data.tags[n].scale);

                float ratio = ((float)overlap)/tag_area;
                if(ratio > .3){
                    ispos = true;
                }
            }
            if(ispos){
                positive_count++; 
                break;
            }else{
                negative_count++;
            }
        }
        detect.clear();
        cout << "Testing with image " << cur << "/" << dataset.size() << endl;
    }
    cout << positive_count << " Positve, " << negative_count << " Negative " << ((float)positive_count/((negative_count!=0)?negative_count: 1)) << "+/-" << endl;
    while(detect.cur_threads() > 0){
        detect.kill_thread();
    }
    //save the svm to output file
    svm->save(argv[3]);
    pthread_exit(NULL);
}
