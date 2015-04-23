jsondir=../dataset_reader/json++
includes=../dataset_reader/DsReader.o
libs=-L $(jsondir)/ -l json
default: tester

tester: tester.cpp $(includes) 
	g++ -g -c -std=c++11 -o tester.o tester.cpp
	g++ -g tester.o $(includes) $(libs) -Wall -o tester

../dataset_reader/DsReader.o: ../dataset_reader/DsReader.cpp
	g++ -std=c++11 -c ../dataset_reader/DsReader.cpp -o ../dataset_reader/DsReader.o

test: tester
	./tester ../../test_images/400x300/tags_final.json test_results.csv ../framework_images/framework_cpp/framework  

test_large: tester
	./tester ../../test_images/1mpx/tags.json test_results_large.csv ../framework_images/framework_cpp/framework
