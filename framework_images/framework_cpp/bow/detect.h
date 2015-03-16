#include <string>
#include <vector>
#include<opencv2/core/core.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/ml/ml.hpp>
#include<opencv2/features2d/features2d.hpp>
#include<opencv2/nonfree/features2d.hpp>
#include<opencv2/nonfree/nonfree.hpp>

#define CLASS "stopsign"
#define MODEL "/home/brenemal/spws/framework_images/framework_cpp/bow/bow_model.yaml" 
#define VOCAB "/home/brenemal/spws/framework_images/framework_cpp/bow/bow.yaml"
/*
* processes image and returns whether image is street sign or not
* Inputs:
*	img - image data
* Outputs:
* 	1 - image contains stop sign
* 	0 - image does not contain stop sign
*/
int processImage(cv::Mat color);
