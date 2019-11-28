# USAGE
# python ball_tracking.py 
# python ball_tracking.py

# import the necessary packages
from collections import deque
import numpy as np
import argparse
import cv2
import time
from matplotlib import pyplot as plt
# construct the argument parse and parse the arguments

roi = cv2.imread("ball5.jpg")
#print roi.shape
hsv_roi = cv2.cvtColor(roi, cv2.COLOR_BGR2HSV)
#print hsv_roi.shape
hue, saturation, value = cv2.split(hsv_roi)
# Histogram ROI
#print hue.shape,hue
#DSA_pixel_M = ((hue >> 4) & 0xF) | ((saturation >>4) & 0xf)
DSA_pixel_M = (hue & 0xF8) | ((saturation >> 5) & 0x7)
M = cv2.calcHist([DSA_pixel_M],[0], None, [256], [0, 256] )

vs = cv2.VideoCapture(0)
#if (cap.isOpened()== False):
#  print("Error opening video  file")
# allow the camera or video file to warm up

# keep looping
while True:
    # grab the current frame
    ret,frame = vs.read()
    if ret == False:
        break


    # handle the frame from VideoCapture or VideoStream

    # if we are viewing a video and we did not grab a frame,
    # then we have reached the end of the video

    hsvt = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
    h,s,v = cv2.split(hsvt)
    #DSA_pixel_I = ((h >>4) & 0xF) | ((s >> 4) & 0xF)
    DSA_pixel_I = (h  & 0xF8) | ((s >> 5) & 0x7)
    I = cv2.calcHist([DSA_pixel_I],[0], None, [256], [0, 256] )

    R = M/I
    # print R.shape
    # print R
    B = R[DSA_pixel_I.ravel()]
    B = np.minimum(B,1)
    # print hsvt.shape[:2]
    B = B.reshape(hsvt.shape[:2])
    kernel = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (5, 5))
    B = cv2.filter2D(B, -1, kernel)
   

    cv2.imshow("Frame", B)
    key = cv2.waitKey(1) & 0xFF

    # if the 'q' key is pressed, stop the loop
    if key == ord("q"):
        break


# otherwise, release the camera
else:
    vs.release()

# close all windows
cv2.destroyAllWindows()
