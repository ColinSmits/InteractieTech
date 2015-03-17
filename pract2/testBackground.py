import numpy as np
import cv2
import functions

cap = cv2.VideoCapture(0)
fgbg = cv2.BackgroundSubtractorMOG()


#frame = cv2.imread("test2OtherLighting.jpg")
#skin filter boundaries (tweak if necessary)
lower = np.array([0, 45, 80], dtype = "uint8")
upper = np.array([11, 255, 255], dtype = "uint8")



#"""
#starting cam: no frame yet
while (True):
    ret, frame1 = cap.read()
    if (not frame1 == None):
        break

#cam started, process frames
while(cap.isOpened):
    ret, frame = cap.read()

    if ret == True:
#"""

		img = frame.copy()
		#back = fgbg.apply(img)
		img = functions.skinFilter(img, lower, upper)
		cv2.imshow("images", img)


#"""      
    k = cv2.waitKey(30) & 0xff
    if k == 27:
       break

cap.release()
"""
cv2.waitKey(0)
"""
cv2.destroyAllWindows()





