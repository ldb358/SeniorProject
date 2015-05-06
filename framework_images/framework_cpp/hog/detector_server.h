#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>

#include<sys/types.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<unistd.h>
#include<netinet/in.h>

#include<opencv2/core/core.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/gpu/gpu.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/ml/ml.hpp>

#include "includes/LinearSVM.h"

using namespace std;
using namespace cv;

#define MODEL "stopsign.yaml" 

struct img_packet{
    int cols;
    int rows;
};

class DetectorServer{
    private:
    	/*
    	 * Variables for object recognition
    	 */
	LinearSVM svm;
	LinearSVM small_svm;
	gpu::HOGDescriptor desc;
	gpu::HOGDescriptor small_desc;
    	int img_count = 0; 
	
	/*
	 * variables for the socket server
	 */
	int sockfd;
	int portno = 9622;

    	void start_socket();	
    public:
	int processImage(Mat &img);
	void wait_for_connections();
	DetectorServer();
};

