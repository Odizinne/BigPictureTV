import subprocess
import time
import pygetwindow as gw
from utils import pyaudiodevices

gamemode_audio = "SAMSUNG"
gamemode_screen = "/external"
desktop_audio = "Headset Earphone"
desktop_screen = "/internal"
gamemode = None
bigpicture_keywords = 'Steam', 'mode', 'Big', 'Picture'

def switch_audio(audio_output):
    devices = pyaudiodevices.get_audio_devices()
    pyaudiodevices.set_audio_device(audio_output, devices)

def switch_screen(mode):
    print(f"Switch screen to {mode}.")
    subprocess.run(["DisplaySwitch.exe", mode])

def startup_reset():
    switch_screen(desktop_screen)
    switch_audio(desktop_audio)

def is_steam_big_picture_open(keywords):
    windows = gw.getAllTitles()
    for window_title in windows:
        if all(word in window_title for word in keywords):
            return True
    return False

startup_reset()

try:
    while True:
        if is_steam_big_picture_open(bigpicture_keywords) and not gamemode:
            switch_screen(gamemode_screen)
            switch_audio(gamemode_audio)
            gamemode = True
        elif not is_steam_big_picture_open(bigpicture_keywords) and gamemode:
            switch_screen(desktop_screen)
            switch_audio(desktop_audio)
            gamemode = False
        time.sleep(1) 
except KeyboardInterrupt:
    print("\nExiting")
