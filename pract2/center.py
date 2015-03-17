import numpy as np
import cv2
import functions

img1 = cv2.imread("testOtherLighting.jpg")
img2 = cv2.imread("test4OtherLighting.jpg")

fgbg = cv2.BackgroundSubtractorMOG()


#frame = cv2.imread("test2OtherLighting.jpg")
#skin filter boundaries (tweak if necessary)
lower = np.array([0, 45, 80], dtype = "uint8")
upper = np.array([11, 255, 255], dtype = "uint8")

list1 = functions.skinFilter(img1, lower, upper)
img1 = list1[0]
center1 = list1[1]
cv2.imshow("image1", img1)  

list2 = functions.skinFilter(img2, lower, upper)
img2 = list2[0]
center2 = list2[1]


cv2.arrowedLine(img2,center1, center2,[0,255,255],2)
cv2.imshow("image2", img2)   


cv2.waitKey(0)

cv2.destroyAllWindows()





