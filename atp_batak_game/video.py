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

# EVENTS

# IDLE      z0
# COUNTDOWN z1
# GAME      z2
# PRE END 	z3
# ENDGAME   z4

capIdle = cv2.VideoCapture('video/standby.mp4') 
capGame = cv2.VideoCapture('video/game.mp4')
capEnd = cv2.VideoCapture('video/ending.mp4') 

cv2.namedWindow('VideoFrame',cv2.WINDOW_NORMAL)
#cv2.moveWindow('VideoFrame',1920,0)
cv2.setWindowProperty('VideoFrame',cv2.WND_PROP_FULLSCREEN,cv2.WINDOW_FULLSCREEN)
cv2.setWindowProperty('VideoFrame', cv2.WND_PROP_TOPMOST, 1)

goIdle = False
goGame = False
goEnd = False

frame_counter = 0

while True:
	try:
		data,add = s.recvfrom(8)
	except:
		pass
	else:
		command = data.decode().split(',')[0]
		value = data.decode().split(',')[1]

		if (command == "z"):
			if (value == "0"):	# STAND-BY

				capIdle.set(cv2.CAP_PROP_POS_FRAMES, 0)
				capGame.set(cv2.CAP_PROP_POS_FRAMES, 0)
				capEnd.set(cv2.CAP_PROP_POS_FRAMES, 0)

				goIdle = True
				goGame = False
				goEnd = False

			if (value == "1"):	# COUNTDOWN
				goIdle = False
				goGame = True
				goEnd = False

			if (value == "2"):	# GAME
				pass
				# cv2.namedWindow('Back',cv2.WINDOW_NORMAL)
				# cv2.moveWindow('Back',1920,0)
				# cv2.setWindowProperty('Back',cv2.WND_PROP_FULLSCREEN,cv2.WINDOW_FULLSCREEN)
				# cv2.setWindowProperty('Back', cv2.WND_PROP_TOPMOST, 0)
				# cv2.setWindowProperty('VideoFrame', cv2.WND_PROP_TOPMOST, 1)

			if (value == "4"):	# END-GAME
				goIdle = False
				goGame = False
				goEnd = True

				# goIdle = False
				# goGame = False
				# cv2.destroyWindow('VideoFrame')

	if goIdle: 
		ret, frame = capIdle.read() 

		if ret:
			frame_counter += 1

			if frame_counter == capIdle.get(cv2.CAP_PROP_FRAME_COUNT):
				frame_counter = 0 
				capIdle.set(cv2.CAP_PROP_POS_FRAMES, 0)

			cv2.imshow('VideoFrame', frame) 
			cv2.waitKey(23)


	if goGame: 
		ret, frame = capGame.read() 

		if ret:
			cv2.imshow('VideoFrame', frame) 
			cv2.waitKey(23)

	if goEnd: 
		ret, frame = capEnd.read() 

		if ret:
			cv2.imshow('VideoFrame', frame) 
			cv2.waitKey(23)


	



