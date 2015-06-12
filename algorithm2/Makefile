default: alg2

alg2: alg2.o
	g++ -g  -Wall alg2.o -o alg2 `pkg-config --cflags --libs opencv`

alg2.o: alg2.cpp
	g++ -g  -c -o alg2.o alg2.cpp


clean:
	rm -f alg2.o alg2 blob.o
