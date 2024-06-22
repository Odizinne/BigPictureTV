import sys
import os
import json
import subprocess
import time
import re
import threading
import pygetwindow as gw
from enum import Enum
from PIL import Image, ImageDraw
from pystray import Icon, MenuItem, Menu
from PyQt5.QtWidgets import QApplication, QMainWindow, QMessageBox
from PyQt5 import uic

class Mode(Enum):
    DESKTOP = 1
    GAMEMODE = 2

tray_icon = None
current_mode = None
constants = None

def generate_default_icon():
    icon_size = (256, 256)
    icon_color = (255, 255, 255, 0)
    icon_image = Image.new('RGBA', icon_size, icon_color)
    draw = ImageDraw.Draw(icon_image)
    draw.ellipse([(50, 50), (200, 200)], fill=(0, 128, 255, 255))
    temp_icon_path = "default_icon.png"
    icon_image.save(temp_icon_path)
    return temp_icon_path

def load_constants():
    appdata_path = os.path.join(os.environ['APPDATA'], "bigpicture-eternal")
    constants_path = os.path.join(appdata_path, 'settings.json')

    if not os.path.exists(appdata_path):
        os.makedirs(appdata_path)

    if not os.path.exists(constants_path):
        create_default_settings(constants_path)

    with open(constants_path, 'r') as f:
        return json.load(f)

def create_default_settings(constants_path):
    settings_template = {
        "BIG_PICTURE_KEYWORDS": ["Steam", "mode", "Big", "Picture"],
        "GAMEMODE_AUDIO": "TV",
        "DESKTOP_AUDIO": "Headset"
    }
    with open(constants_path, 'w') as f:
        json.dump(settings_template, f, indent=4)

    app = QApplication(sys.argv)
    window = SettingsWindow()
    window.show()
    app.exec_()

def read_stream_status():
    file_path = os.path.join(os.environ['APPDATA'], "sunshine-status", "status.txt")
    return os.path.exists(file_path)

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

    if tray_icon:
        tray_icon.menu = create_menu(current_mode)
        tray_icon.update_menu()

def create_menu(current_mode):
    menu_items = [
        MenuItem(f'Current Mode: {current_mode.name}', None),
        MenuItem('GAMEMODE Audio: ' + constants['GAMEMODE_AUDIO'], None),
        MenuItem('DESKTOP Audio: ' + constants['DESKTOP_AUDIO'], None),
        MenuItem('Open Settings', lambda icon, item: open_settings_window_in_thread()),
        MenuItem('Exit', exit_action)
    ]
    return Menu(*menu_items)

def is_bigpicture_running():
    return any(all(word in window_title for word in constants['BIG_PICTURE_KEYWORDS'])
               for window_title in gw.getAllTitles())

def get_mode_file_path():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    return os.path.join(script_dir, 'current_mode.txt')

def write_current_mode(current_mode):
    file_path = get_mode_file_path()

    if current_mode == Mode.GAMEMODE:
        with open(file_path, 'w') as f:
            pass
    elif current_mode == Mode.DESKTOP:
        if os.path.exists(file_path):
            os.remove(file_path)

def read_current_mode():
    file_path = get_mode_file_path()
    if os.path.exists(file_path):
        return Mode.GAMEMODE
    else:
        return Mode.DESKTOP

def exit_action(icon, item):
    icon.stop()
    os._exit(0)

def create_tray_icon(current_mode):
    global tray_icon

    icon_path = os.path.join(os.path.dirname(__file__), 'default_icon.png')
    icon_image = Image.open(icon_path)

    menu = create_menu(current_mode)

    tray_icon = Icon('Sunshine Status', icon=icon_image, menu=menu)

    return tray_icon

def run_tray_icon():
    global tray_icon
    global current_mode
    global constants

    constants = load_constants()

    current_mode = read_current_mode()
    tray_icon = create_tray_icon(current_mode)
    tray_icon.run()

def open_settings_window_in_thread():
    settings_thread = threading.Thread(target=open_settings_window)
    settings_thread.start()

def open_settings_window():
    app = QApplication(sys.argv)
    window = SettingsWindow()
    window.show()
    app.exec_()

class SettingsWindow(QMainWindow):
    def __init__(self):
        super().__init__()

        uic.loadUi('design.ui', self)

        self.constants = load_constants()
        self.load_settings()

        self.saveButton.clicked.connect(self.save_settings)

    def load_settings(self):
        self.steamEntry.setText(' '.join(self.constants['BIG_PICTURE_KEYWORDS']))
        self.desktopEntry.setText(self.constants['DESKTOP_AUDIO'])
        self.gamemodeEntry.setText(self.constants['GAMEMODE_AUDIO'])

    def save_settings(self):
        self.constants['BIG_PICTURE_KEYWORDS'] = self.steamEntry.text().split()
        self.constants['DESKTOP_AUDIO'] = self.desktopEntry.text()
        self.constants['GAMEMODE_AUDIO'] = self.gamemodeEntry.text()

        constants_path = os.path.join(os.environ['APPDATA'], "bigpicture-eternal", 'settings.json')
        try:
            with open(constants_path, 'w') as f:
                json.dump(self.constants, f, indent=4)
            QMessageBox.information(self, 'Success', 'Settings saved successfully.')
            restart_main()
        except Exception as e:
            QMessageBox.critical(self, 'Error', f'Failed to save settings: {e}')

def restart_main():
    python = sys.executable
    os.execl(python, python, *sys.argv)

if __name__ == '__main__':
    GAMEMODE_SCREEN = "/external"
    DESKTOP_SCREEN = "/internal"
    constants = load_constants()
    current_mode = Mode.DESKTOP
    SLEEP_TIME = 1

    if read_current_mode() != Mode.DESKTOP:
        switch_mode(Mode.DESKTOP)

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
