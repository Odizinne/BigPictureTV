import subprocess
import time
import pygetwindow as gw
from utils import pyaudiodevices

BIG_PICTURE_KEYWORDS = ['Steam', 'mode', 'Big', 'Picture']
GAMEMODE_AUDIO = "SAMSUNG"
DESKTOP_AUDIO = "Headset Earphone"
GAMEMODE_SCREEN = "/external"
DESKTOP_SCREEN = "/internal"
GAMEMODE = None

def switch_audio(audio_output):
    devices = pyaudiodevices.get_audio_devices()
    pyaudiodevices.set_audio_device(audio_output, devices)

def switch_screen(mode):
    subprocess.run(["DisplaySwitch.exe", mode])

def startup_reset():
    switch_screen(DESKTOP_SCREEN)
    switch_audio(DESKTOP_AUDIO)

def bigpicture_detector(keywords):
    windows = gw.getAllTitles()
    for window_title in windows:
        if all(word in window_title for word in keywords):
            return True
    return False

startup_reset()

try:
    while True:
        if bigpicture_detector(BIG_PICTURE_KEYWORDS) and not GAMEMODE:
            switch_screen(GAMEMODE_SCREEN)
            switch_audio(GAMEMODE_AUDIO)
            GAMEMODE = True
        elif not bigpicture_detector(BIG_PICTURE_KEYWORDS) and GAMEMODE:
            switch_screen(DESKTOP_SCREEN)
            switch_audio(DESKTOP_AUDIO)
            GAMEMODE = False
        time.sleep(1) 
except KeyboardInterrupt:
    print("\nExiting")
