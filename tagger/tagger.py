#!/usr/bin/env python2
import sys
import os
import cv2

accepted_exts = (".jpg", ".png", ".ppm")

def main():
    if len(sys.argv) < 2:
        rel_path = "."
    else:
        rel_path = sys.argv[1]
        #make sure the path exists
        if not os.path.exists(rel_path):
            print "directory doesnt exist"
            return
    abspath = os.path.abspath(rel_path)
    for root, _, files in os.walk(abspath):
        for f in files:
            if f.endswith(accepted_exts):
                fullpath = os.path.join(root, f)
                img = cv2.imread(fullpath)
                cv2.imshow("test", img)
                cv2.waitKey(1)
if __name__ == "__main__":
    main()
