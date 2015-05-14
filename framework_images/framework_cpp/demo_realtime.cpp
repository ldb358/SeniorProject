/*
 * creates a video that will process a frame as often as possible
 * in the mean time the rectangles are redrawn every frame
 */

#include "demo_realtime.h" // interface for image processing algorithms
using namespace cv;
using namespace std;

#if CLASSIFICATION_ONLY
int processImage(Mat &img, vector<Rect> &rects){
    return 0;
}
#endif
#if DECTECT_ONLY
int processImage(Mat &img){
    return 0;
}
#endif

int main(int argc, char *argv[]){
    if (argc < 3) {
	cout << "Usage: framework video_file output_video" << endl;
	exit(EXIT_FAILURE);
    }
    int sign = 0;
    vector<Rect> rects;

    // load image data  
    VideoCapture in(argv[1]);
    double fps = in.get(CV_CAP_PROP_FPS);
    //the delay between frames
    double delay = 1/fps;
    double width = in.get(CV_CAP_PROP_FRAME_WIDTH);
    double height = in.get(CV_CAP_PROP_FRAME_HEIGHT);

    int frames_left = 0;
    
    VideoWriter out(argv[2], CV_FOURCC('M', 'P', 'E', 'G'), fps, Size(width, height)); 
    Mat img; //the image to read into
    
    int count = 0;
    while(in.read(img)){
    	count++;
    	if(count %50 == 0){
	   cout << "Frame: " << count << endl;
	}
    	if(frames_left <= 0){
	    //timing the process
	    struct timespec start, finish;
	    double delta_t;
	    //start clock
	    clock_gettime(CLOCK_MONOTONIC, &start); 
	    // perform sign recognition
	    // if the image algorithm is a classification algorithm
	    if(CLASSIFICATION_ONLY){
		sign = processImage(img);
		if(sign){
		    rectangle(img, Rect(10, 10, img.cols-20, img.rows-20), Scalar(0, 0, 255), 5); 
		}
	    }else{
		rects.clear();
		sign = processImage(img, rects);
		for(unsigned int i=0; i < rects.size(); ++i){
		    rectangle(img, rects[i], Scalar(0, 0, 255)); 
		}
	    }
	    clock_gettime(CLOCK_MONOTONIC, &finish);
	    //add clock cycles taken to running total
	    delta_t = (finish.tv_sec - start.tv_sec);
	    delta_t += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
	    frames_left = ceil(delta_t/delay);
	}else{
	    frames_left--;
	    if(CLASSIFICATION_ONLY){
		if(sign){
		    rectangle(img, Rect(10, 10, img.cols-20, img.rows-20), Scalar(0, 0, 255), 5); 
		}
	    }else{
		for(unsigned int i=0; i < rects.size(); ++i){
		    rectangle(img, rects[i], Scalar(0, 0, 255)); 
		}
	    }
	}
	
	out << img;	
    }
    return 0;
}

