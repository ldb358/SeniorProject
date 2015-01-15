findColor:	findColor.cpp
	g++ `pkg-config --cflags opencv` -o findColor findColor.cpp `pkg-config --libs opencv` 

videoCaptureExample:	videoCaptureExample.c
	g++ -o videoCaptureExample videoCaptureExample.c

displayCamera_V4L2:	displayCamera_V4L2.cpp
	g++ `pkg-config --cflags opencv` -o displayCamera_V4L2 displayCamera_V4L2.cpp `pkg-config --libs opencv` 

displayCamera:	displayCamera.cpp
	g++ `pkg-config --cflags opencv` -o displayCamera displayCamera.cpp `pkg-config --libs opencv` 

testPattern:	testPattern.cpp
	g++ `pkg-config --cflags opencv` -o testPattern testPattern.cpp `pkg-config --libs opencv` 

framework:	framework.cpp
	g++ `pkg-config --cflags opencv` -o framework framework.cpp `pkg-config --libs opencv` 

clean:
	rm -f videoCaptureExample displayCamera_V4L2 displayCamera testPattern framework
