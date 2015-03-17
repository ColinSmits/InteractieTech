import numpy as np
import cv2

cap = cv2.VideoCapture(0)

fgbg = cv2.BackgroundSubtractorMOG()

while(cap.isOpened):
    ret, frame = cap.read()

    if ret == True:
    	fgmask = fgbg.apply(frame)
        cv2.imshow('frame',fgmask)
    k = cv2.waitKey(30) & 0xff
    if k == 27:
       break

cap.release()
cv2.destroyAllWindows()