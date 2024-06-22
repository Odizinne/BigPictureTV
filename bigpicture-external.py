import subprocess
import time
import re
import os
import json
import pygetwindow as gw
from enum import Enum
from PIL import Image, ImageDraw
from pystray import Icon, MenuItem, Menu
import threading

class Mode(Enum):
    DESKTOP = 1
    GAMEMODE = 2

# Define global variables
tray_icon = None
current_mode = None
constants = None

# Function to generate a default icon
def generate_default_icon():
    icon_size = (256, 256)
    icon_color = (255, 255, 255, 0)  # Fully transparent
    icon_image = Image.new('RGBA', icon_size, icon_color)
    draw = ImageDraw.Draw(icon_image)
    draw.ellipse([(50, 50), (200, 200)], fill=(0, 128, 255, 255))  # Blue circle
    temp_icon_path = "default_icon.png"  # Temporary path for the icon
    icon_image.save(temp_icon_path)
    return temp_icon_path

# Function to load constants from settings.json
def load_constants():
    # Define the AppData path for the settings file
    appdata_path = os.path.join(os.environ['APPDATA'], "bigpicture-eternal")
    constants_path = os.path.join(appdata_path, 'settings.json')

    # Ensure the directory exists
    if not os.path.exists(appdata_path):
        os.makedirs(appdata_path)

    # Check if the settings file exists; if not, create it with the template
    if not os.path.exists(constants_path):
        settings_template = {
            "BIG_PICTURE_KEYWORDS": ["Steam", "mode", "Big", "Picture"],  # Edit these based on your Steam language
            "GAMEMODE_AUDIO": "TV",  # Edit these to your game mode audio device
            "DESKTOP_AUDIO": "Headset"  # Edit these to your desktop mode audio device
        }
        with open(constants_path, 'w') as f:
            json.dump(settings_template, f, indent=4)

    # Load and return the constants from the settings file
    with open(constants_path, 'r') as f:
        return json.load(f)

# Function to read streaming status
def read_stream_status():
    file_path = os.path.join(os.environ['APPDATA'], "sunshine-status", "status.txt")
    return os.path.exists(file_path)

# Function to get audio devices using PowerShell
def get_audio_devices():
    cmd = "powershell Get-AudioDevice -list"
    output = subprocess.check_output(cmd, shell=True, text=True)
    devices = re.findall(r'Index\s+:\s+(\d+)\s+.*?Name\s+:\s+(.*?)\s+ID\s+:\s+{(.*?)}', output, re.DOTALL)
    return devices

# Function to set audio device by name
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

# Function to switch audio output
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

# Function to switch screen mode
def switch_screen(mode):
    subprocess.run(["DisplaySwitch.exe", str(mode)])

# Function to switch mode (desktop or game mode)
def switch_mode(mode):
    global current_mode
    global tray_icon

    if mode == Mode.GAMEMODE:
        switch_screen(GAMEMODE_SCREEN)
        switch_audio(constants['GAMEMODE_AUDIO'])
        current_mode = Mode.GAMEMODE
    else:
        switch_screen(DESKTOP_SCREEN)
        switch_audio(constants['DESKTOP_AUDIO'])
        current_mode = Mode.DESKTOP

    write_current_mode(current_mode)

    # Update the entire menu object
    if tray_icon:
        tray_icon.menu = create_menu(current_mode)
        tray_icon.update_menu()  # Update the tray icon menu

# Function to create the menu
def create_menu(current_mode):
    menu_items = [
        MenuItem(f'Current Mode: {current_mode.name}', None),  # Display current mode
        MenuItem('GAMEMODE Audio: ' + constants['GAMEMODE_AUDIO'], None),
        MenuItem('DESKTOP Audio: ' + constants['DESKTOP_AUDIO'], None),
        MenuItem('Exit', exit_action)
    ]
    return Menu(*menu_items)

# Function to check if Big Picture mode is running
def is_bigpicture_running():
    return any(all(word in window_title for word in constants['BIG_PICTURE_KEYWORDS'])
               for window_title in gw.getAllTitles())

# Function to get current mode file path
def get_mode_file_path():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    return os.path.join(script_dir, 'current_mode.txt')

# Function to write current mode to file
def write_current_mode(current_mode):
    file_path = get_mode_file_path()

    if current_mode == Mode.GAMEMODE:
        with open(file_path, 'w') as f:
            pass
    elif current_mode == Mode.DESKTOP:
        if os.path.exists(file_path):
            os.remove(file_path)

# Function to read current mode from file
def read_current_mode():
    file_path = get_mode_file_path()
    if os.path.exists(file_path):
        return Mode.GAMEMODE
    else:
        return Mode.DESKTOP

# Function to handle exit action
def exit_action(icon, item):
    icon.stop()
    os._exit(0)  # Exit the application

# Function to create tray icon
def create_tray_icon(current_mode):
    global tray_icon

    # Load the icon image using PIL
    icon_path = os.path.join(os.path.dirname(__file__), 'default_icon.png')
    icon_image = Image.open(icon_path)

    # Create the initial menu
    menu = create_menu(current_mode)

    # Create the system tray icon
    tray_icon = Icon('Sunshine Status', icon=icon_image, menu=menu)

    return tray_icon

# Function to run the tray icon in a separate thread
def run_tray_icon():
    global tray_icon
    global current_mode
    global constants

    constants = load_constants()

    current_mode = read_current_mode()  # Read current mode initially
    tray_icon = create_tray_icon(current_mode)
    tray_icon.run()

# Main function
if __name__ == '__main__':
    GAMEMODE_SCREEN = "/external"
    DESKTOP_SCREEN = "/internal"
    constants = load_constants()
    current_mode = Mode.DESKTOP  # Initial mode
    SLEEP_TIME = 1

    if read_current_mode() != Mode.DESKTOP:
        switch_mode(Mode.DESKTOP)

    # Start the tray icon in a separate thread
    tray_thread = threading.Thread(target=run_tray_icon, daemon=True)
    tray_thread.start()

    try:
        while True:
            sunshine_streaming = read_stream_status()
            bigpicture = is_bigpicture_running()
            if bigpicture and current_mode != Mode.GAMEMODE and not sunshine_streaming:
                switch_mode(Mode.GAMEMODE)
            elif not bigpicture and current_mode != Mode.DESKTOP:
                switch_mode(Mode.DESKTOP)
            time.sleep(SLEEP_TIME)
    except KeyboardInterrupt:
        print("\nExiting")
