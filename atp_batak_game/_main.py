import os
import subprocess
import sys
import time
import socket
import serial

try:
    ser = serial.Serial('/dev/ttyACM0',115200)
except:
    try: 
        ser = serial.Serial('/dev/ttyUSB0',115200)
    except:
        print("nessuna seriale trovata")
        sys.exit()

displayA1 = False
displayA2 = False
displayB = True
audio = True

if displayA1:
    subprocess.Popen(["python3","dispA1.py"])
    time.sleep(1)

    SOCK_FILE_A1 = 'sock/displayA1.socket'

    while not os.path.exists(SOCK_FILE_A1):
        print(f"File {SOCK_FILE_A1} doesn't exists")
        time.sleep(1)

    sA1 = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
    sA1.connect(SOCK_FILE_A1)

if displayA2:
    subprocess.Popen(["python3","dispA2.py"])
    time.sleep(1)

    SOCK_FILE_A2 = 'sock/displayA2.socket'

    while not os.path.exists(SOCK_FILE_A2):
        print(f"File {SOCK_FILE_A2} doesn't exists")
        time.sleep(1)

    sA2 = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
    sA2.connect(SOCK_FILE_A2)

if displayB:
    subprocess.Popen(["python3","video.py"])
    time.sleep(1)
    SOCK_FILE_B = 'sock/displayB.socket'

    while not os.path.exists(SOCK_FILE_B):
        print(f"File {SOCK_FILE_B} doesn't exists")
        time.sleep(1)

    sB = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
    sB.connect(SOCK_FILE_B)

if audio:
    subprocess.Popen(["python3","audio.py"])
    time.sleep(1)
    SOCK_FILE_C = 'sock/audio.socket'

    while not os.path.exists(SOCK_FILE_C):
        print(f"File {SOCK_FILE_C} doesn't exists")
        time.sleep(1)

    sC = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
    sC.connect(SOCK_FILE_C)


time.sleep(1)

# TELL THE CONTROLLINO THAT THE GAME HAS STARTED
ser.write("?\r".encode())
ser.flush()

while True:

    if (ser.in_waiting > 2):
        inCh = ser.readline().strip()
        if displayA1:
            sA1.sendall(inCh)
        if displayA2:
            sA2.sendall(inCh)
        if displayB:
            sB.sendall(inCh)
        if audio:
            sC.sendall(inCh)
