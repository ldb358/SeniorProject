#include "detector_server.h"


/*
 * Sets up the detector server and initalizes the SVM
 * and Descriptor
 */
DetectorServer::DetectorServer(){
    //load the model used to classify
    svm.load("/home/sp/SeniorProject/models/stopsign.yaml"); 
    //convert the svm to a vector for floats
    vector<float> support_vector;
    svm.getSupportVector(support_vector);
    //load this into our detector
    desc.setSVMDetector(support_vector);

    start_socket();
}

/*
 * Initlizes the socket server
 */
void DetectorServer::start_socket(){
    struct sockaddr_in server_addr;
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
	throw runtime_error("Error opening socket");
    
    memset((char*)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(portno);
    if(bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    	throw runtime_error("Error binding socket");
    
    listen(sockfd, 10);
}


/*
 * Loops for ever waiting for connections and spawning threads to handle them
 */
void DetectorServer::wait_for_connections(){
    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);
    
    int header_size = sizeof(img_packet);
    struct img_packet packet;

    int clientfd;
    while(1){
	clientfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	if(clientfd < 0)
	    throw runtime_error("Invalid client socket");
	
	memset(&packet, 0, sizeof(packet));
	
	int n = read(clientfd, &packet, header_size);
	if(n != header_size){
	    cout << "Client sent bad packet, ingnoring them." << endl;
	    close(clientfd);
	    continue;
	}
	
	Mat img = Mat::zeros(packet.rows, packet.cols, CV_8UC3);
	int data_size = img.total()*img.elemSize(); 
	
	uchar buffer[data_size];
	int bytes;
	for(int i=0; i < data_size; i += bytes){
	    if((bytes = recv(clientfd, buffer + i, data_size-i, 0)) == -1)
	    	throw runtime_error("Read failed while loading the image");
	}

	int ptr = 0;
	for(int i = 0; i < img.rows; ++i){
	    for(int j = 0; j < img.cols; ++j){
		img.at<Vec3b>(i, j) = Vec3b(buffer[ptr+0], buffer[ptr+1], buffer[ptr+2]);
		ptr += 3;	
	    }
	}
	
	int i = processImage(img);
	write(clientfd, &i, sizeof(int));
	close(clientfd);

    }
}


/*
 * Detects objects that match the model in the image
 * returns > 1 if there was a match found in the image
 */
int DetectorServer::processImage(Mat &img){
    if(img.cols == 0) return 0;    
    Mat gray_img;
    img_count++;
    cvtColor(img, gray_img, CV_BGR2GRAY); 
    
    //our large image holder
    gpu::GpuMat gimg;
    //create the vector to hold the results
    vector<Rect> matches;
    
    //upload our image to the gpu
    gimg.upload(gray_img);
    desc.detectMultiScale(gimg, matches, 0, Size(), Size(0, 0), 1.04, 5);
    for(int i=0; i < matches.size(); ++i){
	rectangle(gray_img, matches[i], Scalar(255, 0, 0));		
    }    
    imwrite("/var/www/training_tests/test"+to_string(img_count)+".jpg", gray_img);

    return matches.size()>0;
}

int main(){
    DetectorServer ds;
    cout << "Starting server!" << endl;
    try{
	ds.wait_for_connections();
    }catch (const exception& e){
	cout << "Server Failed: " << e.what() << endl;
    }

}
