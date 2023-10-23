import os
import socket

# SOCKET

SOCK_FILE = 'sock/displayB.socket'

if os.path.exists(SOCK_FILE):
    os.remove(SOCK_FILE)

s = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
s.bind(SOCK_FILE)
s.setblocking(0)

import cv2 
import numpy as np 
import time


cap = cv2.VideoCapture('video/sample-10s.mp4') 


go = False

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
				cv2.moveWindow('Frame',1920,0)
				cv2.setWindowProperty('Frame',cv2.WND_PROP_FULLSCREEN,cv2.WINDOW_FULLSCREEN)
				cap = cv2.VideoCapture('video/sample-10s.mp4') 
				fps = cap.get(cv2.CAP_PROP_FPS)
				go = False
			if (value == "1"):
				go = True
			if (value == "2"):
				go = False
				cap.release() 
				cv2.destroyAllWindows() 

	if go: 
		ret, frame = cap.read() 

		if ret:
			cv2.imshow('Frame', frame) 
			cv2.waitKey(int(1/fps*1000))


	



