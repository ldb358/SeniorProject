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

    /*
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

    /*
    // Dump contents on later frame to get passed any startup stuff
    if (fnum == 10) {
	// Look in the middle of the image
	fprintf(stderr, "\n");
	for (int y=320; y<324; y++) {
	  fprintf(stderr, " %2d:", y);
	  for (int x=240; x<244; x++) {
	    fprintf(stderr, "    ");
	    Vec3b pix = frame_in.at<Vec3b>(y, x);
	    for (int c=0; c<3; c++) {
	      fprintf(stderr, " %3d", pix.val[c]);
	      }
	    }
	  fprintf(stderr, " : );
	  }
        }
    */

    // Always dump the upper left pixel
    Vec3b pix3b = frame_in.at<Vec3b>(0,0);
    for (int c=0; c<3; c++) {
	fprintf(stderr, " %3d", pix3b.val[c]);
	}
    fprintf(stderr, " : ");

    // Image comes in in signed YCbCr format and we need unsigned YCrCb
    Mat frame_in_flt;
    frame_in.convertTo(frame_in_flt, CV_32F);

    // Always dump the upper left pixel
    Vec3f pix3f = frame_in_flt.at<Vec3f>(0,0);
    for (int c=0; c<3; c++) {
	fprintf(stderr, " %4.1f", pix3f.val[c]);
	}
    fprintf(stderr, " : ");

    Mat frame_uns_ycbcr = frame_in_flt - Scalar_<float>(0.0, 128.0, 128.0);

    // Always dump the upper left pixel
    pix3f = frame_uns_ycbcr.at<Vec3f>(0,0);
    for (int c=0; c<3; c++) {
	fprintf(stderr, " %4.1f", pix3f.val[c]);
	}
    fprintf(stderr, " : ");

    Mat frame_ycrcb = Mat(frame_uns_ycbcr.rows, frame_uns_ycbcr.cols, frame_uns_ycbcr.type());
    int from_to[] = {0,0, 1,2, 2,1};
    mixChannels(&frame_uns_ycbcr, 1, &frame_ycrcb, 1, from_to, 3);

    // Always dump the upper left pixel
    pix3f = frame_ycrcb.at<Vec3f>(0,0);
    for (int c=0; c<3; c++) {
	fprintf(stderr, " %4.1f", pix3f.val[c]);
	}
    fprintf(stderr, " : ");

    Mat frame_bgr;
    cvtColor(frame_ycrcb, frame_bgr, CV_YCrCb2BGR, 0);

    // Always dump the upper left pixel
    pix3f = frame_bgr.at<Vec3f>(0,0);
    for (int c=0; c<3; c++) {
	fprintf(stderr, " %4.1f", pix3f.val[c]);
	}
    fprintf(stderr, "\n");

    // Convert back to 8bit unsigned
    Mat frame_out;
    frame_bgr.convertTo(frame_out, CV_8U);

    // Always dump the upper left pixel
    pix3b = frame_out.at<Vec3b>(0,0);
    for (int c=0; c<3; c++) {
	fprintf(stderr, " %3d", pix3b.val[c]);
	}
    fprintf(stderr, " : ");

    imshow("MyVideo", frame_out);

    if (waitKey(30) == 27) { //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
       fprintf(stderr, "esc key is pressed by user");
       break; 
       }
  fnum ++;
  }

  return 0;
  }
