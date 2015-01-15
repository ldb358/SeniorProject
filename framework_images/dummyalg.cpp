/*
* Dummy computer algorithm that maps a shared memory object containing image
* data to its address space, processes it, and updates whether it is a street
* sign or not.
* Last Modified: 1/11/15
*/
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

using namespace cv;
using namespace std;

// structure to access shared memory
struct imgstruct {
	int sign;	// indicates whether image is sign or not
	uchar img; 	// image data
};

int main(int argc, char *argv[]) 
{
	int fd;
	void *immem;
	struct imgstruct *istruct;
	int size;

	// open and map shared memory object
	if ((fd = shm_open(argv[0], O_RDWR, S_IRUSR | S_IWUSR)) == -1) {
   		perror("Shared memory could not be opened.\n");
		exit(EXIT_FAILURE);
	}
	size = atoi(argv[1]);
	immem = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	istruct = (struct imgstruct *) immem;
	close(fd);

	// process image and update whether image is street sign
	istruct->sign = 0;
	
	return 0;
}
