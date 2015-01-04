#!/usr/bin/env python2
####
# extract_images.py
# this is a script that can be used to extract a sample set of images from a 
# tagged set of images
#
# Usage:
# ./extract_images.py [jsonfile]
# You will then prompted for the class you wish to make the positive sample
# the script will then extract all positives samples in to the [classname]_pos
# directory inside the image directory
# the script will also extract any other tags to the negative samples directory
# [classname]_neg
####
import cv2
import json
import sys
import os

def main():
    num_args = len(sys.argv)
    if num_args < 2:
        print "you must provide a tagger json file to extract images"
        return
    filename = sys.argv[1]
    if not os.path.exists(filename):
        print "file doesnt exist"
        return
    with open(filename, 'r') as json_file:
            jsonstr = json_file.read()
    json_obj = json.loads(jsonstr)
    
    print "Classes:"
    for x, clss in enumerate(json_obj["classes"]):
        print "[%d] %s" % (x, clss)
    
    #get the class we should make the data set for from the user
    index = int(raw_input("Which class would you like to generate a dataset for(0-"+str(len(json_obj["classes"])-1)+ "):"))
    if 0 > index > len(json_obj["classes"])-1:
        print "response not in range"
    
    abspath = json_obj["dataset"]
    if not os.path.exists(abspath):
        print "image directory no longer exists"
        return
    
    #paths to pos and neg directories
    posdir = os.path.join(abspath, json_obj["classes"][index]+"_pos")
    negdir = os.path.join(abspath, json_obj["classes"][index]+"_neg")
    if os.path.exists(posdir) or os.path.exists(negdir):
        print "dataset already created for this class"
        return
    os.mkdir(posdir)
    os.mkdir(negdir)

    width = json_obj["width"]
    height = json_obj["height"]
    for x, image in enumerate(json_obj["images"]):
        img = cv2.imread(image["name"])
        for z, tag in enumerate(image["tags"]):
            x, y = tag["pos"]
            scale = tag["scale"]
            if tag["class"] == index:
                filename = os.path.join(posdir, "img"+str(x)+"tag"+str(z)+".png")
            else:
                filename = os.path.join(negdir, "img"+str(x)+"tag"+str(z)+".png")
            size = img.shape
            x1 = max(min(size[1]-1, x), 0)
            x2 = max(min(size[1]-1, x+scale*width), 0)
            y1 = max(min(size[0]-1, y), 0)
            y2 = max(min(size[0]-1, y+scale*height), 0)
            cropedimg = img[y1:y2, x1:x2]
            scaledimg = cv2.resize(cropedimg, (width, height), interpolation=cv2.INTER_LINEAR)
            cv2.imwrite(filename, scaledimg)

if __name__ == "__main__":
    main()
