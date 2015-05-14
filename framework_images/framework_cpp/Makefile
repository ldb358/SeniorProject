detect_includes=hog/includes/LinearSVM.o hog/includes/SampleDetector.o hog/includes/helper.o 
opencv=`pkg-config --cflags --libs opencv`
cuda=-L /usr/local/cuda/lib/

default: alg.o framework


bow/detect.o: bow/detect.cpp
	g++ -g -std=c++11 -c -o bow/detect.o bow/detect.cpp

hog/detector_client.o: hog/detector.cpp
	g++ -g -std=c++11 -c -o hog/detector_client.o hog/detector_client.cpp


hog/detect.o: hog/detector.cpp
	g++ -g -std=c++11 -c -o hog/detect.o hog/detector.cpp

bow: bow/detect.o framework.o
	g++ -g -c -o framework.o framework.cpp 
	g++ -g framework.o bow/detect.o -Wall -o framework $(opencv)

hog: hog/detect.o framework.o
	g++ -g -c -o framework.o framework.cpp 
	g++ -g framework.o hog/detect.o $(detect_includes) -Wall -o framework $(opencv) $(cuda)

hog_server: hog/detector_client.o
	g++ -g -c -o framework.o framework.cpp
	g++ -g framework.o hog/detector_client.o $(detect_includes) -Wall -o framework $(opencv) $(cuda)

hog_demo_realtime: hog/detector_client.o
	g++ -g -c -o demo_realtime.o demo_realtime.cpp
	g++ -g demo_realtime.o hog/detector_client.o $(detect_includes) -Wall -o demo_realtime $(opencv) $(cuda)

hog_demo_preproc: hog/detector_client.o
	g++ -g -c -o demo_preproc.o demo_preproc.cpp
	g++ -g demo_preproc.o hog/detector_client.o $(detect_includes) -Wall -o demo_preproc $(opencv) $(cuda)

hog_demo_run: hog_demo_realtime hog_demo_preproc 
	./demo_realtime test_videos/short.mpg demo_videos/hog_demo_realtime.mpg 
	./demo_preproc test_videos/short.mpg demo_videos/hog_demo_preproc.mpg 


test: hog_server
	-./framework pos_sample.jpg 
	-./framework neg_sample.jpg

framework: alg.o algorithm.h framework.cpp 
	g++ -g alg.o -o framework framework.cpp $(opencv)

alg.o:	alg.cpp algorithm.h
	g++ `pkg-config --cflags opencv` -g -c alg.cpp  

alg_combine.o:	alg_combine/alg_combine.cpp 
	g++ -g -std=c++11 -c -o alg_combine/alg_combine.o alg_combine/alg_combine.cpp

alg_combine: alg_combine.o framework.o
	g++ -g -c -o framework.o framework.cpp
	g++ -g framework.o alg_combine/alg_combine.o -Wall -o framework $(opencv) $(cuda)

demo_realtime: alg.o demo_realtime.cpp
	g++ -g alg.o demo_realtime.cpp -Wall -o demo_realtime $(opencv) $(cuda)

clean:
	rm -f framework framework_exec dummyalg alg.o

