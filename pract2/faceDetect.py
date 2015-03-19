import numpy as np
import cv2

face_cascade = cv2.CascadeClassifier('casc.xml')


cap = cv2.VideoCapture()
if not cap.open("http://192.168.173.176:8080"):
	print "Didn't work"

#starting cam: no frame yet
while (True):
    ret, frame1 = cap.read()
    if (not frame1 == None):
        break

while(cap.isOpened):
	ret, frame = cap.read()

	if ret == True:
		img = frame.copy()
		# gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
		# faces = face_cascade.detectMultiScale(gray, 1.1 , 5, minSize=(30, 30), flags=cv2.cv.CV_HAAR_SCALE_IMAGE)
		# print len(faces)
		# for (x,y,w,h) in faces:
		#     cv2.rectangle(img,(x,y),(x+w,y+h),(255,0,0),2)
		#     # roi_gray = gray[y:y+h, x:x+w]
		#     # roi_color = img[y:y+h, x:x+w]
		#     # eyes = eye_cascade.detectMultiScale(roi_gray)
		#     # for (ex,ey,ew,eh) in eyes:
		#     #     cv2.rectangle(roi_color,(ex,ey),(ex+ew,ey+eh),(0,255,0),2)

		cv2.imshow('img',img)

	k = cv2.waitKey(30) & 0xff
	if k == 27:
		break

cap.release()
cv2.destroyAllWindows()
