import os
import subprocess
import sys
import time
import socket
import serial

ser = serial.Serial('/dev/ttyACM0',9600)

displayA = True
displayB = True

if displayA:
    subprocess.Popen(["python3","disp2.py"])
    time.sleep(1)

    SOCK_FILE_A = 'sock/displayA.socket'

    while not os.path.exists(SOCK_FILE_A):
        print(f"File {SOCK_FILE_A} doesn't exists")
        time.sleep(1)

    sA = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
    sA.connect(SOCK_FILE_A)

if displayB:
    subprocess.Popen(["python3","video.py"])
    time.sleep(1)
    SOCK_FILE_B = 'sock/displayB.socket'

    while not os.path.exists(SOCK_FILE_B):
        print(f"File {SOCK_FILE_B} doesn't exists")
        time.sleep(1)

    sB = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
    sB.connect(SOCK_FILE_B)


time.sleep(1)

# TELL THE CONTROLLINO THAT THE GAME HAS STARTED
ser.write("?\r".encode())

while True:

    if (ser.in_waiting > 2):
        inCh = ser.readline().strip()
        if displayA:
            sA.sendall(inCh)
        if displayB:
            sB.sendall(inCh)

