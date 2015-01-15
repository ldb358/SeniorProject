default: alg.o framework
	
framework: alg.o algorithm.h framework.cpp 
	g++ `pkg-config --cflags opencv` -g alg.o -o framework framework.cpp `pkg-config --libs opencv` 

alg.o:	alg.cpp algorithm.h
	g++ `pkg-config --cflags opencv` -g -c alg.cpp  

clean:
	rm -f framework framework_exec dummyalg alg.o

