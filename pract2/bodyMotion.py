import numpy as np
import cv2
import functions

import win32com.client as comclt
import win32api

# Initialise Windows shell for communication
wsh= comclt.Dispatch("WScript.Shell")


cap = cv2.VideoCapture(0)
#fgbg = cv2.BackgroundSubtractorMOG()

#skin filter boundaries (tweak if necessary)
lower = np.array([0, 144, 77], dtype = "uint8")
upper = np.array([255, 173, 127], dtype = "uint8")
handFound = False



#starting cam: no frame yet
while (True):
    ret, frame1 = cap.read()
    if (not frame1 == None):
        break

lastFrame = 0
lastFrame2 = 0
lastFound = 0
findDelay = 30
maxDelay = 20
oneHand = False
twoHands = False
trigger = 250
_2HandsFound = False
lastFound2 = 0

trigger2Hands = 200

#cam started, process frames
while(cap.isOpened):
    ret, frame = cap.read()

    if ret == True:

        # Delays in order to make sure a gesture will not be processed twice at once
        if lastFound < findDelay:
            lastFound += 1

        if lastFound2 < findDelay:
            lastFound2 += 1

        if lastFrame2 < findDelay:
            lastFrame2 += 1
        else:
            twoHands = False
            _2HandsFound = False


        img = frame.copy()

        """
        back = fgbg.apply(img)
        img = cv2.bitwise_and(img, img, mask=back)
        kernel = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (3, 3))
        img = cv2.erode(img, kernel, iterations = 2)
        img = cv2.dilate(img, kernel, iterations = 2)
        """

        # process the image to find skin locations
        # Returns list: [ img, centers ] where centers = [(int, int)]
        listImg = functions.skinFilter(img, lower, upper)


        img = listImg[0]
        centers = listImg[1]
        
        
       
        cv2.imshow("img", img)
        

        # one hand found 
        if len(centers) == 1 and not twoHands:
            
            if (handFound == True):
                prev_Offset = functions.processHand(centers[0], prev_centers[0], prev_Offset)
                prev_centers = centers
                length = functions.getLengthX(prev_Offset)
                if abs(length) > trigger:
                    # put Photo Gallery up as active
                    # send the right key 
                    if length < 0: 
                        # previous
                        wsh.AppActivate("Photo Gallery")
                        wsh.sendKeys("{Left}")
                    else:
                        # next 
                        wsh.AppActivate("Photo Gallery")
                        wsh.sendKeys("{Right}")

                    # change state and start delay of recognition
                    handFound = False
                    lastFound = 0

            # change state when not found hand before
            elif lastFound >= findDelay:
                handFound = True
                prev_Offset = (0,0)
                prev_centers = centers

            # make sure state is right
            lastFrame = 0
            oneHand = True



        # 2 hands found
        elif len(centers) == 2:
            
            twoHands = True
            related = []
            lengths = dict()
            if (_2HandsFound == True):
                for center in centers:
                    related.append(functions.getPreviousCenter(center, prev_centers))
                    
                # check whether right centers are appointed
                """
                It's possible this returns the wrong centers. 
                This can only happen when the distance of both is more or less the same. 
                For offset this doesn't matter that much.
                """

                if related[0] == related[1]:
                    print "Kan niet"

                # if centers are reversed: reverse the centers
                if related[0] == 1:
                    centers.reverse()

                # for each center: update the current total offset
                for i in range(len(centers)):
                    
                    prev_Offsets[i] = functions.processHand(centers[i], prev_centers[related[i]], prev_Offsets[i])
                    lengths[i] = functions.getLengthY(prev_Offsets[i])

                # if both offsets are large enough, trigger recognition      
                if abs(lengths[0]) > trigger2Hands and abs(lengths[1]) > trigger2Hands:
                    

                    # check which is left and which is right in order for roation to
                    if centers[0][0] < centers[1][0]:

                        if lengths[0] < 0 and lengths[1] > 0:
                            wsh.AppActivate("Photo Gallery")
                            wsh.sendKeys("^.")
                            _2HandsFound = False
                            lastFound2 = 0

                        elif lengths[1] < 0 and lengths[0] > 0:
                            wsh.AppActivate("Photo Gallery")
                            wsh.sendKeys("^,")
                            _2HandsFound = False
                            lastFound2 = 0

                    elif centers[1][0] < centers[0][0]:

                        if lengths[0] < 0 and lengths[1] > 0:
                            wsh.AppActivate("Photo Gallery")
                            wsh.sendKeys("^,")
                            _2HandsFound = False
                            lastFound2 = 0

                        elif lengths[1] < 0 and lengths[0] > 0:
                            wsh.AppActivate("Photo Gallery")
                            wsh.sendKeys("^.")
                            _2HandsFound = False
                            lastFound2 = 0
                    
                    # nog testen!!
                    """
                    else:
                        prev_Offsets = [(0,0), (0,0)]
                    """

                prev_centers = centers

            # first time when two hands are recognised
            elif lastFound2 >= findDelay:
                _2HandsFound = True
                prev_Offsets = [(0,0), (0,0)]
                prev_centers = centers

            lastFrame2 = 0

        else:
            lastFrame += 1

            if lastFrame > maxDelay:
                handFound == False
                lastFrame = 0
    
    k = cv2.waitKey(30) & 0xff
    if k == 27:
       break

cap.release()
cv2.destroyAllWindows()





