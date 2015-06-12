#include"alg2.hpp"

using namespace cv;
using namespace std;

int main(int argc, char **argv) {
	int threshhold_val;
	Mat src, hsv, eroded, final, corners, corners_norm, edges;
	vector<vector<Point> > contours;

	src = imread(argv[1], 1);
	cvtColor(src, hsv, COLOR_BGR2HSV);
	//imwrite("./original.jpg", src);
	//corners = Mat::zeros(src.size(), CV_32FC1);
	// convert image to filtered red
	hsv = filter_red(hsv);
	imwrite("./hsv.jpg", hsv);

	medianBlur(hsv, final, 7);
	
	//imwrite("./final.jpg", final);
	vector<blob> blobs;
	
	Canny(final, edges, 10, 30, 3);
	//dilate(edges, edges, erosion_element);
	//imwrite("./edges.jpg", edges);

	findContours(edges, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	vector<Point> approx;
	Mat labels = src.clone();

	for(int i = 0; i < contours.size(); i++) {
		approxPolyDP(Mat(contours[i]),
				approx,
				arcLength(Mat(contours[i]), true) * .02,
				true);
		if (fabs(contourArea(contours[i])) < 100 || !isContourConvex(approx))
			continue;
		if(approx.size() == 8){
			printf("Stop sign detected");
			return 0;
		}
	}

	/*
	cornerHarris(final, corners, 20, 19, .02, BORDER_DEFAULT);
	normalize(corners, corners_norm, 0, 255, NORM_MINMAX, CV_32FC1, Mat() );
	imwrite("./corners.jpg", corners_norm);
	for(int y = 0; y < corners_norm.rows; y++) {
		for(int x = 0; x < corners_norm.cols; x++) {
			if((int) corners_norm.at<float>(y,x) > 230) {
				printf("\n%d,%d\n", y, x);
			}
		}
	}
	//vector<set<Point, point_compare> > blobs = get_blobs(final, 25);
	*/
}

Mat filter_red(Mat src) {
	Mat w1, w2, ret_img;
	//Mat ret_img = src.clone();
/*	for(int y = 0; y < src.rows; y++) {
		for(int x = 0; x < src.cols; x++) {
			Vec3b color = src.at<Vec3b>(y, x);
			//eliminate all blue and green
			color[0] = 0;
			color[1] = 0;
			ret_img.at<Vec3b>(y, x) = color;
		}
	}*/
	inRange(src, Scalar(165, 30, 40), Scalar(180, 255, 255), w1);
	//inRange(src, Scalar(0, 30, 40), Scalar(6, 255, 255), w2);
	//addWeighted(w1, 1, w2, 1, 0, ret_img);
	return w1;
}
