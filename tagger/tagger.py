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

    def __str__(self):
        jsondict = {
            "dataset": self.path,
            "width": self.width,
            "height": self.height,
            "images": [x.jsonable() for x in self.images],
            "classes": self.classes
        }
        return json.dumps(jsondict)
    
    @staticmethod
    def from_JSON(jsonstr):
        tagger_json = json.loads(jsonstr)
        tagger = Tagger(tagger_json["dataset"], tagger_json["width"],
                tagger_json["height"])
        tagger.classes = tagger_json["classes"]
        for img in tagger_json["images"]:
            tagger.images.append(Image.from_JSON(img))
        return tagger
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

    def jsonable(self):
        jsondict = {
            "name" : self.name,
            "tags" : [x.jsonable() for x in self.tags]
        }
        return jsondict

    @staticmethod
    def from_JSON(json_obj):
        img = Image(json_obj["name"])
        for tag in json_obj["tags"]:
            img.tags.append(Tag.from_JSON(tag))
        return img
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
        self.x = max(0, x)
        self.y = max(0, y)
    
    def change_scale(self, i):
        self.scale = round(.1*i+.5, 1)
    
    def change_class(self, i):
        self.clss = i

    def jsonable(self):
        jsondict = { 
            "pos": [self.y, self.x], #invert it since python is backwards
            "scale": self.scale,
            "class": self.clss
        }
        return jsondict

    @staticmethod
    def from_JSON(json_obj):
        tag = Tag(json_obj["pos"][0], json_obj["pos"][1], json_obj["scale"], 
                json_obj["class"])
        return tag 

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
        cv2.createTrackbar("scale", "tagging", 9, 9, self.change_scale) 
        cv2.setMouseCallback("tagging", self.tag_image_click)
        #wait for a key press and get the char of the key what was pressed
        key = cv2.waitKey(0) & 255
        while key != ord(" "):
            if key == 27:
                return -1
            key = cv2.waitKey(0) & 255
        self.tagger.cur_image.add_tag(self.tag)
        self.tag.change_class(self.cur_class)

    def change_scale(self, i): 
        if len(self.classes) == 0:
            return

        width = int(self.tagger.width*self.tag.scale)
        height = int(self.tagger.height*self.tag.scale)
        
        self.tag.change_scale(i)
        
        dw = int(self.tagger.width*self.tag.scale)-width
        dh = int(self.tagger.height*self.tag.scale)-height
        size = self.img.shape
        x = max(min(int(self.tag.x-dw/2), size[1]-width-1), 0)
        y = max(min(int(self.tag.y-dh/2), size[0]-height-1), 0)
        self.tag.move(x, y)
        cvimg = self.img.copy()
        self.draw_box(cvimg)
        cv2.imshow("tagging", cvimg)
    
    def tag_image_click(self, event, x, y, flags, param):
        if event == cv2.EVENT_LBUTTONDOWN:
            cvimg = self.img.copy() 
            if self.cur_class > -1:
                width = int(self.tagger.width*self.tag.scale)
                height = int(self.tagger.height*self.tag.scale)
                size = self.img.shape
                x = max(min(x-(width/2), size[1]-width-1), 0)
                y = max(min(y-(height/2), size[0]-height-1), 0)
                self.tag.move(x, y)
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
        self.daemon = True
        self.start()
        self.classes = {}
        self.class_count = 0
        self.max_images = None
        self.entrytext = None
    
    def run(self):
        self.root = tk.Tk()
        self.root.title("test")
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
        classentry.bind("<Return>", self.add_button)
        classentry.grid(column=0, row=1, columnspan=2, sticky="NWES")

        button = tk.Button(self.mainframe, text = 'Add Class', command=self.add_button)
        button.grid(column=0, row=2, columnspan=2, sticky="NWES")

        self.root.mainloop()

    def add_button(self, event=None, text=None):
        clss = self.entrytext.get()
        if text is not None:
            clss = text
        self.entrytext.set("")
        if self.classes.get(clss) == None and not clss.strip() == "":
            self.classes[clss] = self.class_count
            class_count = self.class_count
            button = tk.Button(self.mainframe, text=clss, command=lambda: self.change_class(class_count))
            button.grid(column=0, row=3+class_count, columnspan=2, sticky="NWSE") 
            self.img_tagger.add_class(clss)
            self.change_class(class_count)
            self.class_count += 1
    
    def threadsafe_add_button(self, text):
        while self.entrytext == None:
            pass
        self.add_button(text=text)

    def change_class(self, i):
        self.img_tagger.change_class(i)
        self.labeltext.set(self.img_tagger.classes[i])
    
    def set_title(self, title):
        self.root.title(title)

    def stop(self):
        self.root.quit()

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
    
    tagger = None
   
    skip = 0
    if num_args > 2:
        width, height = [int(x) for x in sys.argv[2:]]
        tagger = Tagger(abspath, width, height) 
    elif os.path.isdir(abspath):
        tagger = Tagger(abspath)
    else:
        #we are instead loading a json file and continuing to tag that dataset
        with open(abspath, 'r') as json_file:
            jsonstr = json_file.read()
        tagger = Tagger.from_JSON(jsonstr)
        abspath = tagger.path
        skip = len(tagger.images)
    
    img_tagger = ImageTagger(tagger)
   
    #set up our gui
    app = TagGui(img_tagger)
    
    #if we are resuming we need to readd our buttons
    for clss in tagger.classes:
        app.threadsafe_add_button(text=clss)
        del tagger.classes[-1]

    #loop through every file in the test dataset directory
    for root, _, files in os.walk(abspath):
        #for each file
        max_images = len(files) 
        n = 0
        for f in files:
            #set the title so we can see how many images to go
            n += 1
            if n <= skip:
                continue
            app.set_title("Image "+str(n)+"/"+str(max_images))
            #make sure the file is a valid image format
            if f.endswith(accepted_exts):
                fullpath = os.path.join(root, f)
                #load the image
                img = cv2.imread(fullpath)
                
                tagger.add_image(fullpath)
                if img_tagger.tag_image(img) == -1:
                    break
                #display the image
                cv2.waitKey(1)
        break
    app.stop()
    print str(tagger)

if __name__ == "__main__":
    main()
