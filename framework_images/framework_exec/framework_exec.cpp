/*
* Framework for testing computer vision algorithms on standalone images
* Last Modified: 1/11/15
*/
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define RENDER
#define NARG 4
#define BUF_SIZE 50
#define MEM_SIZE 2000*2000*8

using namespace cv;
using namespace std;

/*
* Used to access shared memory 
*/
struct imgstruct {
	int sign;
	uchar imgstart;
};

/*
* Creates a shared memory object for image data and sign recognition flag
* Inputs:
* 	path: path of shared memory object
* 	size: size of the shared memory object desired in bytes
*/
void *create_shmem(char *path, int size) {
	int fd;
	void *addr;
	
	fd = shm_open(path, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd == -1)
        {
                printf("failed to open shared memory object.\n");
                exit(EXIT_FAILURE);
        }
        if (ftruncate(fd, size) == -1)
        {
                printf("failed to resize shared memory object.\n");
                exit(EXIT_FAILURE);
        }
        addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (addr == MAP_FAILED)
        {
                printf("failed to map shared memory object.\n");
                exit(EXIT_FAILURE);
        }
	
	return addr;
}

// framework for testing computer vision algorithms on stand-alone images 
int main(int argc, char *argv[]) {
	if (argc < NARG) {
		cout << "Usage: framework image_file mmap_file alg" << endl;
	}
	pid_t pid;
	int status;
	void *signmem;			// shared memory object 
	char *path = argv[2];		// shared memory object path

	// command line arg to computer vision algorithm
	char memstr[BUF_SIZE];
	sprintf(memstr, "%d", MEM_SIZE);

	struct imgstruct *istruct;	// sign recognition structure
	Mat imdata; 			// image data

	// create and map shared memory object	
	signmem = create_shmem(path, MEM_SIZE); 
	istruct = (struct imgstruct *) signmem;

	// load image data into shared memory
	imdata.data = &istruct->imgstart;
	imdata = imread(argv[1], CV_LOAD_IMAGE_COLOR);
	
	
	pid = fork();
	switch(pid) {
	
	case -1:	
		// Error forking
		cerr << "Error: Image processing process could not be created.\n" << endl; 
		if (shm_unlink(path) == -1) {
			cerr << "Error: Problems unlinking shared memory.\n" << endl;
			exit(EXIT_FAILURE);
		}
		exit(EXIT_FAILURE);
		break;

	case 0:
		// child process - computer vision algorithm
		execl(argv[3], argv[2], memstr, (char *)NULL);
		break;
	default:
		// parent process - framework 
		cout << "Waiting for image processing to complete.\n" << endl;
		waitpid(pid, &status, 0);
		break;
	}
	
	// display image
#ifdef RENDER
	cout << "Image processing has completed. Will render image.\n" << endl;
	imshow("opencvtest", imdata);
#endif

	// results of image processing
	istruct->sign ? cout << "This is a stop sign.\n" << endl : cout << "This is not a stop sign.\n" << endl;

#ifdef RENDER
	// press Esc to exit
	waitKey(0);
#endif 

	// unlink shared memory
	if (shm_unlink(path) == -1) {
		cerr << "Error: Problems unlinking shared memory.\n" << endl;
		exit(EXIT_FAILURE);
	}

	return 0;
}
