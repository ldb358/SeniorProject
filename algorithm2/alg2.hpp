#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <set>
#include "blob.cpp"

using namespace cv;
using namespace std;

/* removes all color but red from an image, then converts to greyscale */
Mat filter_red(Mat src);

struct point_compare {
	bool operator() (const Point &lhs, const Point &rhs) {
		if(lhs.x == rhs.x)
			return lhs.y < rhs.y;
		else
			return lhs.x < rhs.x;
	}
};

vector<set<Point, point_compare> > get_blobs(Mat src, int size_thresh);
set<int> get_adjacent_blobs(Point pixel, int** blob_arr);
vector<set<Point, point_compare> > combine_blobs(vector<set<Point, point_compare> > blobs, set<set<int> > blob_combinations);
