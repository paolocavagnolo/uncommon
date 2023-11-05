import os
import socket

# SOCKET

SOCK_FILE = 'sock/audio.socket'

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

import pygame

pygame.init()
pygame.mixer.init()

btn_A = pygame.mixer.Sound("sounds/btn_left.wav")
btn_B = pygame.mixer.Sound("sounds/btn_right.wav")
idle_track = "sounds/standby.wav"
game_track = "sounds/game.wav"
countdown_track = "sounds/countdown2.wav"
end_track = "sounds/ending.wav"

while True:
    try:
        data,add = s.recvfrom(8)
    except:
        pass
    else:
        command = data.decode().split(',')[0]
        value = data.decode().split(',')[1]

        if (command == "z"):
            if (value == "0"):  # STAND-BY
                pygame.mixer.music.load(idle_track)
                pygame.mixer.music.set_volume(0.5) 
                pygame.mixer.music.play(-1)    

            if (value == "1"):  # COUNTDOWN
                pygame.mixer.music.load(countdown_track)
                pygame.mixer.music.set_volume(0.6) 
                pygame.mixer.music.play(-1)  
                
            if (value == "2"):  # GAME
                pygame.mixer.music.load(game_track)
                pygame.mixer.music.set_volume(0.7) 
                pygame.mixer.music.play(-1) 

            if (value == "4"):  # END-GAME
                pygame.mixer.music.load(end_track)
                pygame.mixer.music.set_volume(0.6) 
                pygame.mixer.music.play(-1) 

        if (command == "a"):
            pygame.mixer.Sound.play(btn_A)
        if (command == "b"):
            pygame.mixer.Sound.play(btn_B)
