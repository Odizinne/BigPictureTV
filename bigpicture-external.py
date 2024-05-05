import os
import subprocess
import time
import sys
import psutil
import re
import json
import tkinter as tk
from tkinter import ttk
from tkinter import messagebox
import pygetwindow as gw
import winshell
from win32com.client import Dispatch

CONFIG_FOLDER = os.path.join(os.getenv('LOCALAPPDATA'), 'BigPicture-External')
CONFIG_FILE = os.path.join(CONFIG_FOLDER, "config.json")

DEFAULT_CONFIG = {
    "BIG_PICTURE_KEYWORDS": "Steam Big Picture Mode",
    "GAMEMODE_AUDIO": "My awesome HDMI TV",
    "DESKTOP_AUDIO": "My awesome headset",
    "CLOSE_DISCORD_WHEN_GAMEMODE": False,
    "RUN_AT_STARTUP": False
}

def load_config():
    try:
        with open(CONFIG_FILE, "r") as f:
            return json.load(f)
    except FileNotFoundError:
        return DEFAULT_CONFIG

def save_config(config):
    with open(CONFIG_FILE, "w") as f:
        json.dump(config, f, indent=4)

def toggle_startup_shortcut(enable=True):
    startup_folder = winshell.startup()
    shortcut_path = os.path.join(startup_folder, "BigPictureExternal.lnk")
    
    if enable:
        target = sys.executable
        shell = Dispatch('WScript.Shell')
        shortcut = shell.CreateShortCut(shortcut_path)
        shortcut.Targetpath = target
        shortcut.WorkingDirectory = os.path.dirname(target)
        shortcut.save()
    else:
        if os.path.exists(shortcut_path):
            os.remove(shortcut_path)

def switch_audio(audio_output):
    devices = get_audio_devices()
    set_audio_device(audio_output, devices)

def switch_screen(mode):
    subprocess.run(["DisplaySwitch.exe", mode])

def desktop_reset():
    config = load_config()
    switch_screen("/internal")
    switch_audio(config["DESKTOP_AUDIO"])

def bigpicture_detector(keywords):
    windows = gw.getAllTitles()
    for window_title in windows:
        if all(word in window_title for word in keywords):
            return True
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
            subprocess.run(cmd, shell=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

def kill_discord():
    for proc in psutil.process_iter():
        try:
            if "Discord.exe" in proc.name():
                proc.kill()
                print("Discord process terminated successfully.")
                break
        except psutil.NoSuchProcess:
            pass
    else:
        print("Discord process not found.")

def start_discord():
    subprocess.Popen(["C:\\Users\\Flora\\AppData\\Local\\Discord\\Update.exe", "--processStart", "Discord.exe"])

def apply_changes():
    global CONFIG
    CONFIG = {
        "BIG_PICTURE_KEYWORDS": bigpicture_keywords_entry.get(),
        "GAMEMODE_AUDIO": gamemode_audio_entry.get(),
        "DESKTOP_AUDIO": desktop_audio_entry.get(),
        "CLOSE_DISCORD_WHEN_GAMEMODE": close_discord_var.get(),
        "RUN_AT_STARTUP": run_at_startup_var.get()
    }
    save_config(CONFIG)
    if CONFIG["RUN_AT_STARTUP"]:
        toggle_startup_shortcut(enable=True)
    else:
        toggle_startup_shortcut(enable=False)
    messagebox.showinfo("Saved", "Configuration saved successfully!\nPlease restart the program for changes to take effect.")
    sys.exit()

def create_gui(config):
    global bigpicture_keywords_entry, gamemode_audio_entry, desktop_audio_entry, close_discord_var, run_at_startup_var
    
    root = tk.Tk()
    root.title("Options")
    
    frame = ttk.Frame(root, padding="10")
    frame.grid(row=0, column=0, padx=10, pady=10)
    
    ttk.Label(frame, text="Big Picture window name:").grid(row=0, column=0, sticky="w")
    bigpicture_keywords_entry = ttk.Entry(frame, width=50)
    bigpicture_keywords_entry.grid(row=0, column=1, padx=5, pady=5)
    bigpicture_keywords_entry.insert(0, config["BIG_PICTURE_KEYWORDS"])
    
    ttk.Label(frame, text="Gamemode audio output:").grid(row=1, column=0, sticky="w")
    gamemode_audio_entry = ttk.Entry(frame, width=50)
    gamemode_audio_entry.grid(row=1, column=1, padx=5, pady=5)
    gamemode_audio_entry.insert(0, config["GAMEMODE_AUDIO"])
    
    ttk.Label(frame, text="Desktop audio output:").grid(row=2, column=0, sticky="w")
    desktop_audio_entry = ttk.Entry(frame, width=50)
    desktop_audio_entry.grid(row=2, column=1, padx=5, pady=5)
    desktop_audio_entry.insert(0, config["DESKTOP_AUDIO"])
    
    close_discord_var = tk.BooleanVar()
    ttk.Checkbutton(frame, text="Close Discord in Gamemode", variable=close_discord_var).grid(row=3, column=0, columnspan=2, sticky="w", padx=5, pady=5)
    close_discord_var.set(config["CLOSE_DISCORD_WHEN_GAMEMODE"])
    
    run_at_startup_var = tk.BooleanVar()
    ttk.Checkbutton(frame, text="Run at startup", variable=run_at_startup_var).grid(row=4, column=0, columnspan=2, sticky="w", padx=5, pady=5)
    run_at_startup_var.set(config["RUN_AT_STARTUP"])
    
    ttk.Button(frame, text="Apply Changes", command=apply_changes).grid(row=5, column=0, columnspan=2, pady=10)
    
    root.mainloop()

def main():
    gamemode = False
    if "--gui" in sys.argv or not os.path.exists(CONFIG_FILE):
        config = DEFAULT_CONFIG
        if os.path.exists(CONFIG_FILE):
            config = load_config()
        create_gui(config)
    else:
        desktop_reset()
        config = load_config()
        try:
            while True:
                if bigpicture_detector(config["BIG_PICTURE_KEYWORDS"]) and not gamemode:
                    switch_screen("/external")
                    switch_audio(config["GAMEMODE_AUDIO"])
                    if config["CLOSE_DISCORD_WHEN_GAMEMODE"]:
                        kill_discord()
                    gamemode = True
                elif not bigpicture_detector(config["BIG_PICTURE_KEYWORDS"]) and gamemode:
                    desktop_reset()
                    if config["CLOSE_DISCORD_WHEN_GAMEMODE"]:
                        start_discord()
                    gamemode = False
                time.sleep(1)
        except KeyboardInterrupt:
            print("\nExiting")
        finally:
            sys.exit()

if __name__ == "__main__":
    main()
