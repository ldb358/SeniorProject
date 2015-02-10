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
        clock_t t1,t2; 
        int status = 0;
        //start clock
        t1 = clock();
        //fork and get status
        int pid = fork();
        if(pid == 0){
            //execl(argv[2], argv[2], img_data.path, NULL); 
            return 0;
        }else if(pid < 0){
            cout << "error: could not fork" << endl;
            return 0;
        }
        //wait for the child to stop
        wait(&status);
        t2= clock();
        //add clock cycles taken to running total
        double delta_t = ((float)t2-(float)t1);
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
        myfile << delta_t/CLOCKS_PER_SEC << "," << ((status==ispos)? "correct" : "incorrect") << endl;
    }
    myfile << "Number of Samples" << "," << "Total run time(seconds)" << "," 
        << "average time per image(seconds)" << "," << "total correct" << "," 
        << "total false positive" << "," << "percentage correct" << endl;
    myfile << cur << "," << total_exec_time/CLOCKS_PER_SEC << "," 
        << (total_exec_time/CLOCKS_PER_SEC)/cur << "," << pos_matches << "," 
        << neg_matches  << "," << ((float)pos_matches/cur)*100 << endl;
}
