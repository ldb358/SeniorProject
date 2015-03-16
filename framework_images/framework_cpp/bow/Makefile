jsondir=../../../dataset_reader/json++
includes=../../../dataset_reader/DsReader.o
libs=-L $(jsondir)/ -l json
opencv=`pkg-config --cflags --libs opencv`

default: trainer

trainer: trainer.cpp
	g++ -g -std=c++11 -c -o trainer.o trainer.cpp
	g++ -g trainer.o $(includes) $(libs) -Wall -o trainer $(opencv)


detect: detect.cpp
	g++ -g -std=c++11 -c -o detect.o detect.cpp

train: trainer
	./trainer tags_final.json bow_model.yaml bow.vocab
