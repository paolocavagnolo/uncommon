import os
import socket

# SOCKET

SOCK_FILE = 'sock/displayA2.socket'

if os.path.exists(SOCK_FILE):
    os.remove(SOCK_FILE)

s = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
s.bind(SOCK_FILE)
s.setblocking(0)

# EVENTS

# IDLE      z0
# COUNTDOWN z1
# GAME      z2
# PREEND    z3
# ENDGAME   z4

import cv2 
import numpy as np 
import time

cv2.namedWindow('Image_B_Frame',cv2.WINDOW_NORMAL)
cv2.setWindowProperty('Image_B_Frame',cv2.WND_PROP_FULLSCREEN,cv2.WINDOW_FULLSCREEN)
cv2.setWindowProperty('Image_B_Frame', cv2.WND_PROP_TOPMOST, 1)

blink = False
sT = time.time()
switchImage = False

while True:

    try:
        data,add = s.recvfrom(8)
    except:
        pass
    else:
        print(data.decode())
        command = data.decode().split(',')[0]
        value = data.decode().split(',')[1]

        if (command == "z"):
            if (value == "0"):  # STAND-BY
                blink = False
                name_file = "00"
                img = cv2.imread("img/standby.png")
                cv2.imshow('Image_B_Frame',img)
                cv2.waitKey(3)

            if (value == "1"):  # COUNTDOWN

                img = cv2.imread("img/countdown.png")
                cv2.imshow('Image_B_Frame',img)
                cv2.waitKey(3)
                
            if (value == "2"):  # GAME

                img = cv2.imread("img/00.png")
                cv2.imshow('Image_B_Frame',img)
                cv2.waitKey(3)

            if (value == "3"):  # END-GAME
                sT = time.time()
                blink = True

        if (command == "b"):
            name_file = "{:02d}".format(int(value))
            img = cv2.imread("img/" + name_file + ".png")      
            cv2.imshow('Image_B_Frame',img)
            cv2.waitKey(3)

    if (blink):
        if ((time.time() - sT) > 0.5):
            sT = time.time()
            if (switchImage):
                switchImage = False
                img = cv2.imread("img/" + name_file + ".png")  
                cv2.imshow('Image_B_Frame',img)
                cv2.waitKey(3)
            else:
                switchImage = True
                img = cv2.imread("img/black.png")  
                cv2.imshow('Image_B_Frame',img)
                cv2.waitKey(3)
