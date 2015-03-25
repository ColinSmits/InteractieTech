import cv2
import numpy as np


# Function to detect skin patches and identify their location
def skinFilter(frame, lower, upper):

	# convert frame to YCrCb from BGR (reversed RGB)
	converted = cv2.cvtColor(frame, cv2.COLOR_BGR2YCR_CB)

	#filter the pixels within the skin color range
	skinMask = cv2.inRange(converted, lower, upper)

	# process the frame (erosion, dilation) for better results
	kernel = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (10, 10))
	skinMask = cv2.erode(skinMask, kernel, iterations = 2)
	skinMask = cv2.dilate(skinMask, kernel, iterations = 2)
	skinMask = cv2.GaussianBlur(skinMask, (3,3), 0)

	# Use AND operator to identify pixels that can be used
	skin = cv2.bitwise_and(frame, frame, mask = skinMask)

	#Threshold and create binary image for processing
	skin = cv2.cvtColor(skin, cv2.COLOR_RGB2GRAY)
	ret, tresh1 = cv2.threshold(skin, 0,255,cv2.THRESH_BINARY)
	cv2.imshow("binary", tresh1)

	#calculate the centers of the skin patches by identifying contours
	centers = findDefects(frame, tresh1)
	
	return [frame, centers]



# Function to find the defects and contours in the frame
def findDefects(frame, gray):

	# find the contours
	contours,hierarchy = cv2.findContours(gray, 1, 2)
	maxArea = 0
	ci = 0
	
	n = 0
	cx = None
	cy = None
	ci = []
	centers = []
	if len(contours) > 1:

		# for each contour: calculate the area and filter the patches that are too small
		for i in range(len(contours)):
			cnt = contours[n]
			area = cv2.contourArea(cnt)
					
			if area > 4000:
				ci.append(i)

			n += 1
		
		# for each contour left:
		# - calculate the COG by using the moments of the contour (http)
		# - Get the defects of the fingers
		# - By defining the defect location: draw lines between the fingertips
		# 	creating a contour of the hand
		# - Draw the contour of the hand on the frame

		for i in ci:
			cnt = contours[i]
			M = cv2.moments(cnt)
			cx = int(M['m10']/M['m00'])
			cy = int(M['m01']/M['m00'])
			centers.append((cx, cy))
			hull = cv2.convexHull(cnt,returnPoints = False)
			defects = cv2.convexityDefects(cnt,hull)

			for i in range(defects.shape[0]):
				s,e,f,d = defects[i,0]
				start = tuple(cnt[s][0])
				end = tuple(cnt[e][0])
				far = tuple(cnt[f][0])
				cv2.line(frame,start,end,[0,255,0],2)
				cv2.circle(frame,far,5,[0,0,255],-1)

	return centers
			

# Function to calculate the total offset given the current and previous centers, 
# and the cumulative offset yet obtained
def processHand(center, prev_center, prev_Offset):


	difference_X = center[0] - prev_center[0]
	difference_y = center[1] - prev_center[1]

	off_X = prev_Offset[0] + difference_X
	off_Y = prev_Offset[1] + difference_y

	return (off_X, off_Y) 


# Functions for length processing
def getLength(vector):
	return (vector[0]**2 + vector[1]**2)**0.5


def getLengthX(vector):
	return vector[0]

def getLengthY(vector):
	return vector[1]



# Function to get the right associations between two centers in two consecutive frames
# (Only when two hands are in the frame)
def getPreviousCenter(center, previous_centers):
	i = 0
	maxDistance = 10000000
	#print "Previous: %s" % (previous_centers)
	#print "Current: %s" % (str(center))
	for prev_center in previous_centers:

		difference_X = center[0] - prev_center[0]
		difference_Y = center[1] - prev_center[1]

		totalDifference = abs(difference_X)

		if totalDifference < maxDistance:
			maxDistance = totalDifference
			current = i

		i += 1

	#print "Related to center %s: %s = %s" % (str(center), current, previous_centers[current])

	return current	

