jsondir=includes/json++
includes=includes/FileReader.o
libs=-L $(jsondir)/ -l json
default: trainer


trainer: trainer.cpp $(includes)
	g++ -std=c++11 -c -o trainer.o trainer.cpp
	g++ trainer.o $(includes) $(libs) -Wall -o trainer `pkg-config --cflags --libs opencv`

includes/FileReader.o: includes/FileReader.cpp
	g++ -c includes/FileReader.cpp -o includes/FileReader.o

$(jsondir)/libjson.a:
	make -f $(jsondir)/Makefile

run: trainer
	touch puck.model
	rm puck.model
	./trainer tagger.json ~/test_images/rover/puck.txt puck.model

clean:
	rm *.o trainer includes/*.o
