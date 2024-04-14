import subprocess
import time
import pygetwindow as gw

subprocess.run(["DisplaySwitch.exe", "/clone"])
subprocess.run(["powershell", "-Command", f"& {{ Set-AudioDevice -ID '{{0.0.0.00000000}}.{{1f765a44-226e-4d89-a7a7-fdd617a46670}}' }}"])

def is_steam_big_picture_open(keywords):

    windows = gw.getAllTitles()

    for window_title in windows:
        if all(word in window_title for word in keywords):
            return True
    
    return False

steam_big_picture_keywords = ['Steam', 'mode', 'Big', 'Picture']
big_picture_running = None

try:
    while True:
        if is_steam_big_picture_open(steam_big_picture_keywords) and not big_picture_running:
            print("Steam Big Picture mode is open.")
            subprocess.run(["DisplaySwitch.exe", "/external"])
            subprocess.run(["powershell", "-Command", f"& {{ Set-AudioDevice -ID '{{0.0.0.00000000}}.{{0d4b032e-75b2-4a96-b321-9573ce0b5beb}}' }}"])
            big_picture_running = True
        elif not is_steam_big_picture_open(steam_big_picture_keywords) and big_picture_running:
            print("Steam Big Picture mode is not open.")
            subprocess.run(["DisplaySwitch.exe", "/clone"])
            subprocess.run(["powershell", "-Command", f"& {{ Set-AudioDevice -ID '{{0.0.0.00000000}}.{{1f765a44-226e-4d89-a7a7-fdd617a46670}}' }}"])
            big_picture_running = False
        time.sleep(1) 
except KeyboardInterrupt:
    print("\nExiting")