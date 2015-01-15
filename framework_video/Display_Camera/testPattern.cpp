#include <opencv2/opencv.hpp>

using namespace cv;

int main( int argc, char** argv ) {

  namedWindow("MyVideo",CV_WINDOW_AUTOSIZE);

  Mat img = Mat::zeros(480, 640, CV_8UC3);

  rectangle(img, Point(  0,0), Point( 79,479), Scalar(255, 0, 0), CV_FILLED);
  rectangle(img, Point( 80,0), Point(159,479), Scalar(0, 255, 0), CV_FILLED);
  rectangle(img, Point(160,0), Point(239,479), Scalar(0, 0, 255), CV_FILLED);

  imshow("MyVideo", img);

  waitKey(0);

  return 0;
  }
