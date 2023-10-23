import pyglet
import pyglet.window.key
import os
import socket


# SOCKET

SOCK_FILE = 'sock/displayA.socket'

if os.path.exists(SOCK_FILE):
    os.remove(SOCK_FILE)

s = socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM)
s.bind(SOCK_FILE)


# DISPLAY

width = 500
height = 500

display = pyglet.canvas.get_display()

screen = display.get_screens()[0]
window = pyglet.window.Window(width, height, "Title1", screen = screen)

pic = pyglet.image.load("img/s2.jpg")
sprite = pyglet.sprite.Sprite(img=pic)


# AUDIO
sample1 = pyglet.media.StaticSource(pyglet.media.load('sounds/sample-1s.wav'))


# EVENTS

# IDLE      z0
# COUNTDOWN z1
# GAME      z2
# ENDGAME   z3

def update(dt):
    data,add = s.recvfrom(8)
    print(data)
    command = data.decode()[0]
    value = data.decode()[1]

    if (command == "z"):
        if (value == "0"):
            window.clear()
        if (value == "1"):
            sprite.draw()
        if (value == "2"):
            sample1.play()
        if (value == "3"):
            window.clear()


pyglet.clock.schedule_interval(update, 1/30)


# RUN

window.set_fullscreen(True)
pyglet.app.run()
