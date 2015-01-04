#!/usr/bin/env python2
import sys
import os
import cv2
import Tkinter as tk
import threading
import json

accepted_exts = (".jpg", ".png", ".ppm")

#
# This is the height level of the tagger data wrapper class
# args: 
# abspath - path to the directory that the images are in
# width -  the width of output images
# height - the height of output images
#
class Tagger(object):

    def __init__(self, abspath=".", width=200, height=200):
        self.path = abspath
        self.width = width
        self.height = height
        self.images = []
        self.classes = []
        self.cur_image = None

    def cur(self):
        return self.cur_image

    #appends a new images object
    def add_image(self, filename):
        self.images.append(Image(filename))
        self.cur_image = self.images[-1]
    
    def add_class(self, name):
        self.classes.append(name)


#
# this class is a data wrapper for the Image object in the JSON Api
# it takes a filename of the image to init
#
class Image(object):
    def __init__(self, filename):
        self.name = filename
        self.tags = []

    #appends a new tag object
    def add_tag(self, tag):
        self.tags.append(tag)        
        return self.tags[-1]

#
# Data wrapper for the Tag JSON data type
# args:
# x - x pos of the top left corner of the tag in the original image
# y - y pos of the top left corner of the tag
# w - width of the tag box
# h - height of the tag box
# positive - whether the tag is a posative or negative sample
# path - Path to the sliced out image
#

class Tag(object):
    def __init__(self, x, y, scale, clss):
        self.x = x
        self.y = y
        self.scale = scale
        self.clss = clss

    def move(self, x, y, scale=1):
        self.x = x
        self.y = y
    
    def change_scale(self, i):
        self.scale = round(.1*i+.1, 1)
    
    def change_class(self, i):
        self.clss = i

    def __str__(self):
        jsondict = { 
            "pos": [self.x, self.y],
            "scale": self.scale,
            "class": self.clss
        }
        return json.dumps(jsondict);
#
# This class handles the callbacks for the two different guis to modify our
# tag data classes
#

class ImageTagger(object):

    def __init__(self, tagger):
        self.classes = []
        self.cur_class = -1
        self.tagger = tagger
        self.tag = None
        self.img = None

    def tag_image(self, cvimg):
        self.img = cvimg
        cv2.imshow("tagging", cvimg)
        self.tag = Tag(0, 0, 1, self.cur_class)
        self.tagger.cur_image.add_tag(self.tag)
        cv2.createTrackbar("scale", "tagging", 9, 9, self.change_scale) 
        cv2.setMouseCallback("tagging", self.tag_image_click)
        #wait for a key press and get the char of the key what was pressed
        key = cv2.waitKey(0) & 255
        while key != ord(" "):
            key = cv2.waitKey(0) & 255
        self.tag.change_class(self.cur_class)

    def change_scale(self, i): 
        if len(self.classes) == 0:
            return

        width = int(self.tagger.width*self.tag.scale)
        height = int(self.tagger.height*self.tag.scale)
        
        self.tag.change_scale(i)
        
        dw = int(self.tagger.width*self.tag.scale)-width
        dh = int(self.tagger.height*self.tag.scale)-height
        self.tag.move(int(self.tag.x-dw/2), int(self.tag.y-dh/2))
        cvimg = self.img.copy()
        self.draw_box(cvimg)
        cv2.imshow("tagging", cvimg)
    
    def tag_image_click(self, event, x, y, flags, param):
        if event == cv2.EVENT_LBUTTONDOWN:
            cvimg = self.img.copy() 
            if self.cur_class > -1:
                width = int(self.tagger.width*self.tag.scale)
                height = int(self.tagger.height*self.tag.scale)
                self.tag.move(x-(width/2), y-(height/2))
                self.draw_box(cvimg)
            else:
                cv2.putText(cvimg, "You must select an object type.", (x, y), cv2.FONT_HERSHEY_PLAIN, 1, 9)
            cv2.imshow("tagging", cvimg) 

    def draw_box(self, cvimg):
        width = int(self.tagger.width*self.tag.scale)
        height = int(self.tagger.height*self.tag.scale)
        cv2.rectangle(cvimg, (self.tag.x, self.tag.y),(self.tag.x+width, self.tag.y+height), (255, 0, 0))
        size = cv2.getTextSize(self.classes[self.cur_class], cv2.FONT_HERSHEY_PLAIN, 1, 9)
        cv2.putText(cvimg, self.classes[self.cur_class], (self.tag.x+width/2-size[0][0]/2, self.tag.y+height/2-size[0][1]/2), cv2.FONT_HERSHEY_PLAIN, 1, 9)


    def add_class(self, clss):
        self.classes.append(clss)
        self.tagger.add_class(clss)

    def change_class(self, i):
        self.cur_class = i

#
# This class handles the tkgui, this gui is run in a seperate thread so that 
# highgui can also run properly.
#
class TagGui(threading.Thread):
    def __init__(self, img_tagger):
        threading.Thread.__init__(self)
        self.img_tagger = img_tagger
        self.start()
        self.classes = {}
        self.class_count = 0
    
    def run(self):
        self.root = tk.Tk()

        self.mainframe = tk.Frame(self.root)
        self.mainframe.grid(column=0, row=0, sticky="NWES")
       
        curclasslabel = tk.Label(self.mainframe, text="Current Class:", justify="right")
        curclasslabel.grid(column=0, row=0, sticky="NWES")

        self.labeltext = tk.StringVar()
        curclass = tk.Label(self.mainframe, textvariable=self.labeltext, justify="left")
        curclass.grid(column=1, row=0, sticky="NSEW")
        self.labeltext.set("None Selected")
        
        self.entrytext = tk.StringVar()
        classentry = tk.Entry(self.mainframe, textvariable=self.entrytext)
        classentry.grid(column=0, row=1, columnspan=2, sticky="NWES")

        button = tk.Button(self.mainframe, text = 'Add Class', command=self.add_button)
        button.grid(column=0, row=2, columnspan=2, sticky="NWES")

        self.mainframe.bind("<Return>", self.add_button)

        self.root.mainloop()

    def add_button(self):
        clss = self.entrytext.get()
        self.entrytext.set("")
        if self.classes.get(clss) == None and not clss.strip() == "":
            self.classes[clss] = self.class_count
            class_count = self.class_count
            button = tk.Button(self.mainframe, text=clss, command=lambda: self.change_class(class_count))
            button.grid(column=0, row=3+class_count, columnspan=2, sticky="NWSE") 
            self.img_tagger.add_class(clss)
            self.change_class(class_count)
            self.class_count += 1
            
    def change_class(self, i):
        self.img_tagger.change_class(i)
        self.labeltext.set(self.img_tagger.classes[i])


def main():
    num_args = len(sys.argv)
    if num_args < 2:
        rel_path = "."
    else:
        rel_path = sys.argv[1]
        #make sure the path exists
        if not os.path.exists(rel_path):
            print "directory doesnt exist"
            return
    abspath = os.path.abspath(rel_path)
    
    build = False
    tagger = None
    
    if num_args > 2:
        if sys.argv[2] == "build":
            if num_args < 5:
                print "To build a dataset you must provied a width and height for the test images"
                return
            width, height = [int(x) for x in sys.argv[3:]]

            #get the path to the positve and negitive sample directories
            pospath = os.path.join(abspath, "pos")
            negpath = os.path.join(abspath, "neg")
            try:
                #create the pos and negitive dirs     
                os.mkdir(pospath)
                os.mkdir(negpath)
            except OSError:
                print "pos and neg already exited, we will populate these"
            build = True
            tagger = Tagger(abspath, width, height, pospath, negpath) 
    else:
        tagger = Tagger(abspath)
    
    
    img_tagger = ImageTagger(tagger)
   
    #set up our gui
    app = TagGui(img_tagger)

    #loop through every file in the test dataset directory
    for root, _, files in os.walk(abspath):
        #for each file
        for f in files:
            #make sure the file is a valid image format
            if f.endswith(accepted_exts):
                fullpath = os.path.join(root, f)
                #load the image
                img = cv2.imread(fullpath)
                
                tagger.add_image(fullpath)
                img_tagger.tag_image(img)
                for i in tagger.images:
                    for x in i.tags:
                        print str(x)
                #display the image
                cv2.waitKey(1)


if __name__ == "__main__":
    main()
