svmlightdir = ~/svmlight/
svmlight = $(svmlightdir)svm_learn.o $(svmlightdir)svm_common.o $(svmlightdir)svm_hideo.o
default: trainer


trainer: trainer.cpp FileReader.o
	g++ -c -o trainer.o trainer.cpp
	g++ trainer.o FileReader.o -o trainer `pkg-config --cflags --libs opencv`

FileReader.o: FileReader.cpp
	g++ -c FileReader.cpp -o FileReader.o

run: trainer
	touch puck.model
	rm puck.model
	./trainer ~/test_images/rover/puck.txt puck.model

clean:
	rm *.o trainer includes/*.o
