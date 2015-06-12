default: alg1

alg1: alg1.o
	g++ -g  -Wall alg1.o -o alg1 `pkg-config --cflags --libs opencv`

alg1.o: alg1.cpp
	g++ -g  -c -o alg1.o alg1.cpp


clean:
	rm -f alg1.o alg1 blob.o
