import subprocess
import time
import pygetwindow as gw

BIG_PICTURE_KEYWORDS = ['Steam', 'mode', 'Big', 'Picture']
GAMEMODE_SCREEN = "/external"
DESKTOP_SCREEN = "/internal"
gamemode = None

def switch_screen(mode):
    subprocess.run(["DisplaySwitch.exe", mode])

def startup_reset():
    switch_screen(DESKTOP_SCREEN)

def bigpicture_detector(keywords):
    windows = gw.getAllTitles()
    for window_title in windows:
        if all(word in window_title for word in keywords):
            return True
    return False

startup_reset()

try:
    while True:
        if bigpicture_detector(BIG_PICTURE_KEYWORDS) and not gamemode:
            switch_screen(GAMEMODE_SCREEN)
            gamemode = True
        elif not bigpicture_detector(BIG_PICTURE_KEYWORDS) and gamemode:
            switch_screen(DESKTOP_SCREEN)
            gamemode = False
        time.sleep(1) 
except KeyboardInterrupt:
    print("\nExiting")
