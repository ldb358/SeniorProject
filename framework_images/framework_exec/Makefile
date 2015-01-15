default: framework_exec dummyalg
	
framework_exec:	framework_exec.cpp
	g++ `pkg-config --cflags opencv` -g -o framework_exec framework_exec.cpp `pkg-config --libs opencv` 

dummyalg: dummyalg.cpp
	g++ `pkg-config --cflags opencv` -g -o dummyalg dummyalg.cpp `pkg-config --libs opencv` 

clean:
	rm -f framework_exec dummyalg

