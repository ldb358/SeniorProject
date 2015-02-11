#include "detector.h"


int processImage(Mat img){
    //arguments
    int step_size = STEP_SIZE;
   
    //the cv class used to extract the hog descriptors
    HOGDescriptor desc;

    //the svm we are going to use
    CvSVM svm;
    //load the model used to classify
    svm.load(MODEL);
    //create an instance of our sample detection class
    SampleDetector detect(&svm, WINDOW_WIDTH, WINDOW_HEIGHT);
    //set up our detection threads
    pthread_t threads[NUM_THREADS];
    //create and init thread attribute
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    //set join
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    //spawn our threads
    for(int i=0; i < NUM_THREADS; ++i){
        int rc = pthread_create(&threads[i], &attr, SampleDetector::start_thread, (void*)&detect);
        if(rc){
            cout << "Pthread failed to create threads with error code:" << rc << endl;
            return 0;
        }
    }
    //clean up attribute
    pthread_attr_destroy(&attr);
    //set the image for our thread 
    detect.set_img(img);
    //for each row tell a thread to work on it
    for(int i=0, max_rows=img.rows-WINDOW_HEIGHT; i < max_rows; i+=step_size){
        //this function will block until a worker picks it up
        detect.scan_row_threaded(desc, i, step_size);
    }
    //wait for workers to finish
    while(detect.tworking()){}
    vector<struct SdMatch> matches = detect.get_matches();

    return (matches.size()>0);
}
