svmlightdir = ~/svmlight/
svmlight = $(svmlightdir)svm_learn.o $(svmlightdir)svm_common.o $(svmlightdir)svm_hideo.o
default: trainer


trainer: trainer.cpp
	g++ trainer.cpp -o trainer `pkg-config --cflags --libs opencv` $(svmlight) 
	
run: trainer
	touch puck.model
	rm puck.model
	./trainer ~/test_images/rover/puck.txt puck.model
