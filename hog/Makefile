default: trainer

trainer: trainer.cpp
	g++ trainer.cpp -o trainer `pkg-config --cflags --libs opencv` 
	
run: trainer
	./trainer ~/test_images/rover/puck.txt
