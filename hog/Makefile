jsondir=../dataset_reader/json++
includes=includes/FileReader.o includes/SampleDetector.o includes/helper.o ../dataset_reader/DsReader.o includes/LinearSVM.o
detect_includes=includes/SampleDetector.o includes/helper.o 
libs=-L $(jsondir)/ -l json
opencv=`pkg-config --cflags --libs opencv`
cuda=-L /usr/local/cuda/lib/
default: trainer


trainer: trainer.cpp $(includes)
	g++ -g -pthread -std=c++11 -c -o trainer.o trainer.cpp
	g++ -g -std=c++11 trainer.o $(includes) $(libs) $(cuda) -Wall -o trainer $(opencv) 

libhogdetect.so: detector.cpp $(detect_includes)
	g++ -g -pthread -std=c++11 -fPIC -c -o detector.o detector.cpp 
	g++ -std=c++11 -c -fPIC includes/SampleDetector.cpp -o includes/SampleDetector.o
	g++ -fPIC -c includes/helper.cpp -o includes/helper.o
	g++ -shared -o libhogdetect.so detector.o $(detect_includes) $(cuda) $(opencv)


includes/FileReader.o: includes/FileReader.cpp
	g++ -c includes/FileReader.cpp -o includes/FileReader.o

../dataset_reader/DsReader.o: ../dataset_reader/DsReader.cpp
	g++ -std=c++11 -c ../dataset_reader/DsReader.cpp -o ../dataset_reader/DsReader.o

includes/LinearSVM.o: includes/LinearSVM.cpp
	g++ -c includes/LinearSVM.cpp -o includes/LinearSVM.o


includes/SampleDetector.o: includes/SampleDetector.cpp
	g++ -std=c++11 -c includes/SampleDetector.cpp -o includes/SampleDetector.o

includes/helper.o: includes/helper.cpp
	g++ -c includes/helper.cpp -o includes/helper.o


$(jsondir)/libjson.a:
	make -f $(jsondir)/Makefile

rover: trainer
	touch puck.yaml
	rm puck.yaml
	./trainer tagger.json ~/test_images/rover/puck.txt puck.yaml

stop: trainer
	touch stopsign.yaml
	rm stopsign.yaml
	./trainer ~/test_images/400x300/tags_final.json ~/test_images/400x300/stopsign.txt stopsign.yaml

clean:
	rm *.o trainer includes/*.o
