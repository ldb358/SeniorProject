#include <stdio.h>
#include <opencv2/opencv.hpp>

using namespace cv;

int main( int argc, char** argv ) {
  VideoCapture cap(0);
  if (!cap.isOpened()) {
    fprintf(stderr, "Cannot open the video file");
    return -1;
    }

  cap.set(CV_CAP_PROP_FRAME_WIDTH,  640);
  cap.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
  cap.set(CV_CAP_PROP_FOURCC, CV_FOURCC('B', 'G', 'R', '3'));

  // These don't work
  // fprintf(stderr, "cap.fourcc = %d\n", int(cap.get(CV_CAP_PROP_FOURCC)));
  // fprintf(stderr, "cap.format = %d\n", int(cap.get(CV_CAP_PROP_FORMAT)));
  // fprintf(stderr, "cap.cvtRGB = %d\n", int(cap.get(CV_CAP_PROP_CONVERT_RGB)));

  namedWindow("MyVideo",CV_WINDOW_AUTOSIZE);

  int fnum = 0;

  while (1) {
    Mat frame_in;
    bool bSuccess = cap.read(frame_in); // read a new frame from video

    if (!bSuccess) {
       fprintf(stderr, "Cannot read a frame from video file");
       break;
       }

    // Dump info on first frame
    if (fnum == 0) {
	int mat_type = frame_in.type();
	int mat_depth = frame_in.depth();
	int mat_channels = frame_in.channels();
        fprintf(stderr, "mat type = %d, depth = %d, channels = %d\n", 
		mat_type, mat_depth, mat_channels);
        fprintf(stderr, "image size = %d x %d, flags = 0x%x\n", frame_in.cols, frame_in.rows, frame_in.flags);
        fprintf(stderr, "elemSize = %d, channels = %d, elemSize1 = %d\n", frame_in.elemSize(), frame_in.channels(), frame_in.elemSize1());
        fprintf(stderr, "depth = %d\n", frame_in.depth());
        }

    /*
    // Dump 
    fprintf(stderr, "\n");
    for (int y=0; y<4; y++) {
      fprintf(stderr, " %2d:", y);
      for (int x=0; x<8; x++) {
        fprintf(stderr, "    ");
        Vec3b pix = frame_in.at<Vec3b>(y, x);
        for (int c=0; c<3; c++) {
          fprintf(stderr, " %3x", pix.val[c]);
          }
        }
      fprintf(stderr, "\n");
      }
    */

    // Display wants BGR so swap
    Mat frame_bgr = Mat(frame_in.rows, frame_in.cols, frame_in.type());
    int from_to[] = {0,0, 1,2, 2,1};
    mixChannels(&frame_in, 1, &frame_bgr, 1, from_to, 3);

    imshow("MyVideo", frame_bgr);

    if (waitKey(30) == 27) { //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
       fprintf(stderr, "esc key is pressed by user");
       break; 
       }
  fnum ++;
  }

  return 0;
  }
