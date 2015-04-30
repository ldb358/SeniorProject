#include "DsReader.h"
using namespace std;
using namespace JSON;
/*
 * DsReader: Reads a json data set file created by tagger.py
 */
DsReader::DsReader(char *filename){
    status = 0;
    try{
        root = parse_file(filename);
        status = 1;   
    }catch(std::runtime_error e){
        status = 0;
    }
    if(status != 0){
        twidth = root["width"].as_int();
        theight = root["height"].as_int();
        Array clsses = (Array)root["classes"];
        for(int i=0; i < clsses.size(); ++i){
            classes.push_back((string)clsses[i]);
        }

    }
    path_buffer = new char[400];
    current = 0;
}

/*
 * Returns the next image tag object
 */
int DsReader::has_next(){
    return ((Array)root["images"]).size() > current;
}

/*
 * Returns the number of images
 */
int DsReader::size(){
    return ((Array)root["images"]).size();
}


/*
 * can be used to check if there was any data in the file
 */
int DsReader::data_exists(){
    return status;
}

/*
 * takes a struct by reference and populates it for the next image then increments
 * the counter
 */
void DsReader::next(struct DsImage &imgdata){
    Object element = root["images"][current];
    //get the path
    string path = (string)element["name"];
    imgdata.path = path_buffer;
    strcpy(imgdata.path, path.c_str());
    
    imgdata.tags.clear();
    //get the tags and convert them to a struct
    struct DsTag tag;
    Array tags = (Array)element["tags"];
    for(int i=0; i < tags.size(); ++i){
        //use a wrapper struct for the pos(since vector doesn't like arrays)
        struct DsPos temp_pos = {0, 0};
        //cast it to an int so we can access it like an array
        int *int_pos = &temp_pos.x;
        Array pos = (Array)tags[i]["pos"]; 
        for(int j=0; j < pos.size(); ++j){
            int_pos[j] = pos[j].as_int();
        }
        tag.pos = temp_pos;
        tag.scale = atof(tags[i]["scale"].as_string().c_str());
        tag.clss = tags[i]["class"].as_int();
    }
    imgdata.tags.push_back(tag);
    current++;
}

/*
 * resets the couuter in the dataset
 */
void DsReader::reset(){
    current = 0;
}


