detect_includes=hog/includes/LinearSVM.o hog/includes/SampleDetector.o hog/includes/helper.o 
opencv=`pkg-config --cflags --libs opencv`
cuda=-L /usr/local/cuda/lib/

default: alg.o framework


bow/detect.o: bow/detect.cpp
	g++ -g -std=c++11 -c -o bow/detect.o bow/detect.cpp

hog/detect.o: hog/detector.cpp
	g++ -g -std=c++11 -c -o hog/detect.o hog/detector.cpp

bow: bow/detect.o framework.o
	g++ -g -c -o framework.o framework.cpp 
	g++ -g framework.o bow/detect.o -Wall -o framework $(opencv)

hog: hog/detect.o framework.o
	g++ -g -c -o framework.o framework.cpp 
	g++ -g framework.o hog/detect.o $(detect_includes) -Wall -o framework $(opencv) $(cuda)


test: hog
	-./framework pos_sample.jpg 
	-./framework neg_sample.jpg

framework: alg.o algorithm.h framework.cpp 
	g++ -g alg.o -o framework framework.cpp $(opencv)

alg.o:	alg.cpp algorithm.h
	g++ `pkg-config --cflags opencv` -g -c alg.cpp  

clean:
	rm -f framework framework_exec dummyalg alg.o

