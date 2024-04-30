import subprocess
import time
import pygetwindow as gw
import json
import os

CONFIG_FILE = "config.json"

def read_config(file_path):
    with open(file_path, 'r') as config_file:
        config = json.load(config_file)
    return config

def write_default_config(file_path):
    default_config = {
        "//": "Please specify the audio output device names in this config file",
        "gamemode_audio": "",
        "desktop_audio": ""
    }
    with open(file_path, 'w') as config_file:
        json.dump(default_config, config_file, indent=4)

def set_audio_device(audio_output):
    subprocess.call(["pythonw", "pyaudiodevices.py", "-d", audio_output])

def is_steam_big_picture_open(keywords):
    windows = gw.getAllTitles()
    for window_title in windows:
        if all(word in window_title for word in keywords):
            return True
    return False

if not os.path.exists(CONFIG_FILE):
    print("Configuration file doesn't exist. Creating default configuration...")
    write_default_config(CONFIG_FILE)
    print("Default configuration created in 'config.json'. Please edit this file with your audio outputs.")
    exit()

if os.stat(CONFIG_FILE).st_size == 0:
    print("Configuration file is empty. Script cannot continue without audio settings.")
    exit()

config = read_config(CONFIG_FILE)
gamemode_audio = config["gamemode_audio"]
desktop_audio = config["desktop_audio"]
gamemode = None
try:
    while True:
        if is_steam_big_picture_open(['Steam', 'mode', 'Big', 'Picture']) and not gamemode:
            print("Steam Big Picture mode is open.")
            subprocess.run(["DisplaySwitch.exe", "/external"])
            set_audio_device(gamemode_audio)
            gamemode = True
        elif not is_steam_big_picture_open(['Steam', 'mode', 'Big', 'Picture']) and gamemode:
            print("Steam Big Picture mode is not open.")
            subprocess.run(["DisplaySwitch.exe", "/internal"])
            set_audio_device(desktop_audio)
            gamemode = False
        time.sleep(1) 
except KeyboardInterrupt:
    print("\nExiting")
