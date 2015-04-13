#include "tester.h"
using namespace std;

int main(int argc, char**argv){
    if(argc < 3){
        cout << "Usage Format: ./tester [path to json file] [csv file to save results to] [path to executable]" << endl;
        return 0;
    }
    
    DsReader *dataset = new DsReader(argv[1]);
    if(!dataset->data_exists()){
        cout << "Error: " << argv[1] << " doesn't exist! Cannot test without a dataset." << endl;
    }

    ofstream myfile;
    myfile.open (argv[2], ios::out | ios::trunc);


    struct DsImage img_data;

    int pos_matches = 0;
    int neg_matches = 0;
    double total_exec_time = 0.0;
    int cur = 0;
    myfile << "Run Time" << "," << "Correctness" << endl;
    while(dataset->has_next()){
        dataset->next(img_data);
        cur++;
        struct timespec start, finish;
        double delta_t;
        //start clock
        clock_gettime(CLOCK_MONOTONIC, &start); 
        int status = 0;
        //fork and get status
        int pid = fork();
        if(pid == 0){
            execl(argv[3], argv[3], img_data.path, NULL); 
        }else if(pid < 0){
            cout << "error: could not fork" << endl;
            return 0;
        }
        //wait for the child to stop
        wait(&status);
        //end clock
        clock_gettime(CLOCK_MONOTONIC, &finish);
        //add clock cycles taken to running total
        delta_t = (finish.tv_sec - start.tv_sec);
        delta_t += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;

        total_exec_time += delta_t;
        int ispos = 0;
        //foreach tag if tag.class == "stopsign"
        for(int i=0; i < img_data.tags.size(); ++i){
            string cls = dataset->get_class(img_data.tags[i].clss);
            if(cls == "stopsign"){
                ispos = 1;
            }
        }
        if(ispos == status){
            pos_matches++;
        }else{
            neg_matches++;
        }
        cout << "Testing with image " << cur << " Expected:" << (ispos? "pos" : "neg")
            << " Got:" << (status? "pos" : "neg") << endl;
        myfile << delta_t << "," << ((status==ispos)? "correct" : "incorrect") << endl;
    }
    myfile << "Number of Samples" << "," << "Total run time(seconds)" << "," 
        << "average time per image(seconds)" << "," << "total correct" << "," 
        << "total false positive" << "," << "percentage correct" << endl;
    myfile << cur << "," << total_exec_time << "," 
        << (total_exec_time/cur) << "," << pos_matches << "," 
        << neg_matches  << "," << ((float)pos_matches/cur)*100 << endl;
}
