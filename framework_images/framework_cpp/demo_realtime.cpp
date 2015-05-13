/*
 * creates a video that will process a frame as often as possible
 * in the mean time the rectangles are redrawn every frame
 */
#include "demo_realtime.h" // interface for image processing algorithms
using namespace cv;
using namespace std;

#ifdef CLASSIFICATION_ONLY
int processImage(Mat &img, vector<Rect> &rects){
    return 0;
}
#endif
#ifdef DECTECT_ONLY
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
    // load image data  
    VideoCapture in(argv[1]);
    double fps = in.get(CV_CAP_PROP_FPS);
    double width = in.get(CV_CAP_PROP_FRAME_WIDTH);
    double height = in.get(CV_CAP_PROP_FRAME_HEIGHT);

    VideoWriter out(argv[2], CV_FOURCC('M', 'P', 'G', '4'), fps, Size(width, height)); 
    Mat img;
    while(in.read(img)){
	// perform sign recognition
	// if the image algorithm is a classification algorithm
	#ifdef CLASSIFICATION_ONLY
	    sign = processImage(img);
	    if(sign){
		rectangle(img, Rect(10, 10, img.rows-20, img.cols-20), Scalar(0, 0, 255)); 
	    }
	#else
	    vector<Rect> rects;
	    sign = processImage(img, rects);
	    for(unsigned int i=0; i < rects.size(); ++i){
		rectangle(img, rects[i], Scalar(0, 0, 255)); 
	    }
	#endif
	out << img;	
    }
    return sign;
}

