default: alg.o framework


bow/detect.o: bow/detect.cpp
	g++ -g -std=c++11 -c -o bow/detect.o bow/detect.cpp

bow: bow/detect.o
	g++ -g -c -o framework.o framework.cpp 
	g++ -g framework.o bow/detect.o -Wall -o framework `pkg-config --cflags --libs opencv`

framework: alg.o algorithm.h framework.cpp 
	g++ `pkg-config --cflags opencv` -g alg.o -o framework framework.cpp `pkg-config --libs opencv` 

alg.o:	alg.cpp algorithm.h
	g++ `pkg-config --cflags opencv` -g -c alg.cpp  

clean:
	rm -f framework framework_exec dummyalg alg.o

