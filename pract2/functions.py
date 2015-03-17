import cv2
import numpy as np

def skinFilter(frame, lower, upper):
	converted = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
	skinMask = cv2.inRange(converted, lower, upper)
	kernel = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (11, 11))
	skinMask = cv2.erode(skinMask, kernel, iterations = 3)
	skinMask = cv2.dilate(skinMask, kernel, iterations = 3)
	skinMask = cv2.GaussianBlur(skinMask, (3,3), 0)
	skin = cv2.bitwise_and(frame, frame, mask = skinMask)
	skin = cv2.cvtColor(skin, cv2.COLOR_RGB2GRAY)
	ret, tresh1 = cv2.threshold(skin, 0,255,cv2.THRESH_BINARY)
	center = findDefects(frame, tresh1)
	
	return [frame, center]


def findDefects(frame, gray):
	contours,hierarchy = cv2.findContours(gray, 1, 2)
	maxArea = 0
	ci = 0
	
	n = 0
	if (contours):
		for i in range(len(contours)):
			cnt = contours[n]
			#if cv2.isContourConvex(cnt):
			print "True"
			print "\n"
			area = cv2.contourArea(cnt)
			print area
			print "\n"
			if area > maxArea:
				maxArea = area
				ci = n

			n += 1
		
		#print str(contours)
		cnt = contours[ci]
		
		M = cv2.moments(cnt)
		cx = int(M['m10']/M['m00'])
		cy = int(M['m01']/M['m00'])
		#print len(contours)
		#print ci
		#print str(cnt)
		hull = cv2.convexHull(cnt,returnPoints = False)
		#print str(hull)
		defects = cv2.convexityDefects(cnt,hull)
		#print str(defects)

		for i in range(defects.shape[0]):
			s,e,f,d = defects[i,0]
			start = tuple(cnt[s][0])
			end = tuple(cnt[e][0])
			far = tuple(cnt[f][0])
			#print start
			#print end
			#print far
			cv2.line(frame,start,end,[0,255,0],2)
			cv2.circle(frame,far,5,[0,0,255],-1)

	return (cx, cy)
			








