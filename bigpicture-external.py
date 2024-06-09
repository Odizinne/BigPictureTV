import subprocess
import time
import re
import os
import json
import pygetwindow as gw
from enum import Enum

class Mode(Enum):
    DESKTOP = 1
    GAMEMODE = 2


def load_constants():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    constants_path = os.path.join(script_dir, 'settings.json')
    with open(constants_path, 'r') as f:
        return json.load(f)


def read_stream_status():
    file_path = os.path.join(os.environ['APPDATA'], "sunshine-status", "status.txt")
    if os.path.exists(file_path):
        with open(file_path, "r") as file:
            content = file.read().strip().lower()
            return content == "true"
    return False


def get_audio_devices():
    cmd = "powershell Get-AudioDevice -list"
    output = subprocess.check_output(cmd, shell=True, text=True)
    devices = re.findall(r'Index\s+:\s+(\d+)\s+.*?Name\s+:\s+(.*?)\s+ID\s+:\s+{(.*?)}', output, re.DOTALL)
    return devices


def set_audio_device(device_name, devices):
    device_words = device_name.lower().split()

    for index, name, _ in devices:
        lower_name = name.lower()

        if all(word in lower_name for word in device_words):
            cmd = f"powershell set-audiodevice -index {index}"
            result = subprocess.run(cmd, shell=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

            if result.returncode == 0:
                return True
            else:
                return False
    return False


def switch_audio(audio_output):
    devices = get_audio_devices()
    success = set_audio_device(audio_output, devices)

    retries = 0
    while not success and retries < 10:
        print("Failed to switch audio, retrying...")
        time.sleep(1)
        success = set_audio_device(audio_output, devices)
        retries += 1

    if not success:
        print("Failed to switch audio after 10 attempts.")


def switch_screen(mode):
    subprocess.run(["DisplaySwitch.exe", str(mode)])


def switch_mode(mode):
    if mode == Mode.GAMEMODE:
        switch_screen(GAMEMODE_SCREEN)
        switch_audio(GAMEMODE_AUDIO)
    else:
        switch_screen(DESKTOP_SCREEN)
        switch_audio(DESKTOP_AUDIO)


def is_bigpicture_running():
    return any(all(word in window_title for word in BIG_PICTURE_KEYWORDS)
               for window_title in gw.getAllTitles())

def write_current_mode(current_mode):
    script_dir = os.path.dirname(os.path.abspath(__file__))
    file_path = os.path.join(script_dir, 'current_mode.txt')
    with open(file_path, 'w') as f:
        f.write(str(current_mode.value))

def read_current_mode():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    file_path = os.path.join(script_dir, 'current_mode.txt')
    if not os.path.exists(file_path):
        with open(file_path, 'w') as f:
            f.write(str(Mode.DESKTOP.value))
            return Mode.DESKTOP
    else:
        with open(file_path, 'r') as f:
            mode_value = int(f.read())
            return Mode(mode_value)
    
constants = load_constants()
BIG_PICTURE_KEYWORDS = constants['BIG_PICTURE_KEYWORDS']
GAMEMODE_SCREEN = constants['GAMEMODE_SCREEN']
DESKTOP_SCREEN = constants['DESKTOP_SCREEN']
GAMEMODE_AUDIO = constants['GAMEMODE_AUDIO']
DESKTOP_AUDIO = constants['DESKTOP_AUDIO']
RETRY_LIMIT = 10
SLEEP_TIME = 1
current_mode = Mode.DESKTOP

if read_current_mode() != Mode.DESKTOP:
    switch_mode(Mode.DESKTOP)

try:
    while True:
        sunshine_streaming = read_stream_status()
        bigpicture = is_bigpicture_running()
        if bigpicture and current_mode != Mode.GAMEMODE and not sunshine_streaming:
            switch_mode(Mode.GAMEMODE)
            current_mode = Mode.GAMEMODE 
            write_current_mode(current_mode)
        elif not bigpicture and current_mode != Mode.DESKTOP:
            switch_mode(Mode.DESKTOP)
            current_mode = Mode.DESKTOP
            write_current_mode(current_mode)
        time.sleep(SLEEP_TIME)
except KeyboardInterrupt:
    print("\nExiting")
