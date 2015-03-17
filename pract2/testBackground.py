import numpy as np
import cv2

imgBack = cv2.imread("background.jpg", 0)
imgFore = cv2.imread("inserted.jpg", 0)

fgbg = cv2.BackgroundSubtractorMOG()

fgmask = fgbg.apply(imgBack)
bgmask = fgbg.apply(imgFore)

cv2.imshow('frame', fgmask)
cv2.waitKey(0)
cv2.imshow('frame', bgmask)
cv2.waitKey(0)
cv2.destroyAllWindows()