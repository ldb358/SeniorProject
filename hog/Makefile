jsondir=includes/json++
includes=includes/FileReader.o includes/SampleDetector.o includes/helper.o includes/DsReader.o
libs=-L $(jsondir)/ -l json
default: trainer


trainer: trainer.cpp $(includes)
	g++ -g -pthread -std=c++11 -c -o trainer.o trainer.cpp
	g++ -g trainer.o $(includes) $(libs) -Wall -o trainer `pkg-config --cflags --libs opencv`

includes/FileReader.o: includes/FileReader.cpp
	g++ -c includes/FileReader.cpp -o includes/FileReader.o

includes/DsReader.o: includes/DsReader.cpp
	g++ -std=c++11 -c includes/DsReader.cpp -o includes/DsReader.o

includes/SampleDetector.o: includes/SampleDetector.cpp
	g++ -std=c++11 -c includes/SampleDetector.cpp -o includes/SampleDetector.o

includes/helper.o: includes/helper.cpp
	g++ -c includes/helper.cpp -o includes/helper.o


$(jsondir)/libjson.a:
	make -f $(jsondir)/Makefile

run: trainer
	touch puck.model
	rm puck.model
	./trainer tagger.json ~/test_images/rover/puck.txt puck.yaml

clean:
	rm *.o trainer includes/*.o
