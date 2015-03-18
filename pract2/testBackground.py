import numpy as np
import cv2
import functions

import win32com.client as comclt
import win32api

# Initialiseer de Windows shell in Python zodat je naar de shell kunt scripten
wsh= comclt.Dispatch("WScript.Shell")


cap = cv2.VideoCapture(0)
fgbg = cv2.BackgroundSubtractorMOG()


#frame = cv2.imread("test2OtherLighting.jpg")
#skin filter boundaries (tweak if necessary)
lower = np.array([0, 45, 80], dtype = "uint8")
upper = np.array([15, 255, 255], dtype = "uint8")
handFound = False


#"""
#starting cam: no frame yet
while (True):
    ret, frame1 = cap.read()
    if (not frame1 == None):
        break

lastFrame = 0
lastFound = 0
findDelay = 20
maxDelay = 20

trigger = 400

#cam started, process frames
while(cap.isOpened):
    ret, frame = cap.read()

    if ret == True:

        if lastFound < findDelay:
            lastFound += 1

        img = frame.copy()
       # back = fgbg.apply(img)
       # img = cv2.bitwise_and(img, img, mask=back)
       # kernel = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (3, 3))
       # img = cv2.erode(img, kernel, iterations = 2)
       # img = cv2.dilate(img, kernel, iterations = 2)
        listImg = functions.skinFilter(img, lower, upper)
        img = listImg[0]
        center = listImg[1]
        cv2.imshow("images", img)

        if (center[0] != None):
            if (handFound == True):
                prev_Offset = functions.processHand(center, prev_center, prev_Offset)
                
                length = functions.getLengthX(prev_Offset)
                if abs(length) > trigger:
                    print length
                    if length < 0:
                        wsh.AppActivate("Photo Gallery") # select another application
                        wsh.sendKeys("{Left}")
                    else:
                        wsh.AppActivate("Photo Gallery")
                        wsh.sendKeys("{Right}")

                    handFound = False
                    lastFound = 0


            elif lastFound >= findDelay:
                handFound = True
                prev_Offset = (0,0)
                prev_center = center

            lastFrame = 0

        else:
            lastFrame += 1

            if lastFrame > maxDelay:
                handFound == True





#"""      
    k = cv2.waitKey(30) & 0xff
    if k == 27:
       break

cap.release()
"""
cv2.waitKey(0)
"""
cv2.destroyAllWindows()





