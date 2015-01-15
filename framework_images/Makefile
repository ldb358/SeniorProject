default: alg.o framework framework_exec dummyalg
	
framework: alg.o algorithm.h framework.cpp 
	g++ `pkg-config --cflags opencv` -g alg.o -o framework framework.cpp `pkg-config --libs opencv` 

alg.o:	alg.cpp algorithm.h
	g++ `pkg-config --cflags opencv` -g -c alg.cpp  

framework_exec:	framework_exec.cpp
	g++ `pkg-config --cflags opencv` -g -o framework_exec framework_exec.cpp `pkg-config --libs opencv` 

dummyalg: dummyalg.cpp
	g++ `pkg-config --cflags opencv` -g -o dummyalg dummyalg.cpp `pkg-config --libs opencv` 

clean:
	rm -f framework framework_exec dummyalg alg.o

