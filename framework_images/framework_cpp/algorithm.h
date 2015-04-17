// interface to framework 

#include <opencv2/highgui/highgui.hpp>

using namespace cv;

/*
* processes image and returns whether image is street sign or not
* Inputs:
*	img - image data
* Outputs:
* 	1 - image contains stop sign
* 	0 - image does not contain stop sign
*/
int processImage(Mat &img);
