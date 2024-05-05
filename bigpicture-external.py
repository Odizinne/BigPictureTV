import subprocess
import time
import pygetwindow as gw

BIG_PICTURE_KEYWORDS = ['Steam', 'mode', 'Big', 'Picture']
GAMEMODE_SCREEN = "/external"
DESKTOP_SCREEN = "/internal"
gamemode = False

def switch_screen(mode):
    subprocess.run(["DisplaySwitch.exe", mode], check=True)

def startup_reset():
    switch_screen(DESKTOP_SCREEN)

def is_bigpicture_running():
    windows = gw.getAllTitles()
    for window_title in windows:
        if all(word in window_title for word in BIG_PICTURE_KEYWORDS):
            return True
    return False

startup_reset()

try:
    while True:
        bigpicture = is_bigpicture_running()
        if bigpicture and not gamemode:
            switch_screen(GAMEMODE_SCREEN)
            gamemode = True
        elif not bigpicture and gamemode:
            switch_screen(DESKTOP_SCREEN)
            gamemode = False
        time.sleep(1) 
except KeyboardInterrupt:
    print("\nExiting")
