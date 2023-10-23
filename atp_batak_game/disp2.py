import os
import socket

# SOCKET

SOCK_FILE = 'sock/displayA.socket'

if os.path.exists(SOCK_FILE):
    os.remove(SOCK_FILE)

s = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
s.bind(SOCK_FILE)
s.setblocking(0)


import cv2 
import numpy as np 
import time

while True:
    try:
        data,add = s.recvfrom(8)
    except:
        pass
    else:
        command = data.decode()[0]
        value = data.decode()[1]

        if (command == "z"):
            if (value == "0"):
                cv2.namedWindow('Frame',cv2.WINDOW_NORMAL)
                cv2.setWindowProperty('Frame',cv2.WND_PROP_FULLSCREEN,cv2.WINDOW_FULLSCREEN)
                # cv2.moveWindow('Frame',0,0)
                img = cv2.imread("img/s2.jpg")
            if (value == "1"):
                cv2.imshow('Frame',img)
                cv2.waitKey(3)
            if (value == "2"):
                cv2.destroyAllWindows()



