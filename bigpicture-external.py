import subprocess
import time
import re
import pygetwindow as gw

BIG_PICTURE_KEYWORDS = ['Steam', 'mode', 'Big', 'Picture']
GAMEMODE_SCREEN = "/external"
DESKTOP_SCREEN = "/internal"
GAMEMODE_AUDIO = "SAMSUNG"
DESKTOP_AUDIO = "Headset Earphone"
gamemode = False

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
            subprocess.run(cmd, shell=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

def switch_audio(audio_output):
    devices = get_audio_devices()
    set_audio_device(audio_output, devices)

def switch_screen(mode):
    subprocess.run(["DisplaySwitch.exe", mode])

def is_bigpicture_running():
    return any(all(word in window_title for word in BIG_PICTURE_KEYWORDS) 
               for window_title in gw.getAllTitles())

switch_screen(DESKTOP_SCREEN)

try:
    while True:
        bigpicture = is_bigpicture_running()
        if bigpicture and not gamemode:
            switch_screen(GAMEMODE_SCREEN)
            time.sleep(1)
            switch_audio(GAMEMODE_AUDIO)
            gamemode = True
        elif not bigpicture and gamemode:
            switch_screen(DESKTOP_SCREEN)
            switch_audio(DESKTOP_AUDIO)
            gamemode = False
        time.sleep(1) 
except KeyboardInterrupt:
    print("\nExiting")
