import sys
import os
import json
import subprocess
import time
import re
import threading
import winshell
import pygetwindow as gw
from enum import Enum
from PIL import Image
from pystray import Icon, MenuItem, Menu
from PyQt5.QtWidgets import QApplication, QMainWindow, QDialog, QMessageBox
from PyQt5.QtGui import QIcon
from PyQt5.QtCore import Qt
from PyQt5 import uic
import winreg

class Mode(Enum):
    DESKTOP = 1
    GAMEMODE = 2

GAMEMODE_SCREEN = "/external"
DESKTOP_SCREEN = "/internal"
SETTINGS_FILE = os.path.join(os.environ['APPDATA'], "bigpicture-eternal", "settings.json")
UI_FOLDER = os.path.join(os.path.dirname(__file__), 'ui')
ICONS_FOLDER = os.path.join(os.path.dirname(__file__), 'icons')

tray_icon = None
current_mode = None
constants = None

def load_constants():
    if not os.path.exists(os.path.dirname(SETTINGS_FILE)):
        os.makedirs(os.path.dirname(SETTINGS_FILE))

    if not os.path.exists(SETTINGS_FILE):
        create_default_settings()

    with open(SETTINGS_FILE, 'r') as f:
        return json.load(f)

def create_default_settings():
    settings_template = {
        "BIG_PICTURE_KEYWORDS": ["Steam", "mode", "Big", "Picture"],
        "GAMEMODE_AUDIO": "TV",
        "DESKTOP_AUDIO": "Headset",
        "UseSystemTheme": False
    }
    with open(SETTINGS_FILE, 'w') as f:
        json.dump(settings_template, f, indent=4)

def read_stream_status():
    file_path = os.path.join(os.environ['APPDATA'], "bigpicture-eternal", "sunshine-status", "status.txt")
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
            return result.returncode == 0
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
        MenuItem(f'Mode: {current_mode.name}', None, enabled=False),
        MenuItem('', None, enabled=False, visible=False),
        MenuItem('Settings', lambda icon, item: open_settings_window_in_thread()),
        MenuItem('Exit', exit_action)
    ]
    return Menu(*menu_items)

def is_bigpicture_running():
    return any(all(word in window_title for word in constants['BIG_PICTURE_KEYWORDS'])
               for window_title in gw.getAllTitles())

def get_mode_file_path():
    script_dir = getattr(sys, '_MEIPASS', os.path.dirname(os.path.abspath(__file__)))
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
    return Mode.GAMEMODE if os.path.exists(file_path) else Mode.DESKTOP

def exit_action(icon, item):
    icon.stop()
    os._exit(0)

def create_tray_icon(current_mode):
    global tray_icon

    use_system_theme = constants.get('UseSystemTheme', False)
    icon_file = 'steamos-logo.png'

    if use_system_theme:
        try:
            reg_path = r'SOFTWARE\Microsoft\Windows\CurrentVersion\Themes\Personalize'
            with winreg.OpenKey(winreg.HKEY_CURRENT_USER, reg_path) as key:
                theme_type = winreg.QueryValueEx(key, 'AppsUseLightTheme')[0]
                if theme_type == 1:
                    icon_file = 'steamos-logo-dark.png'
                else:
                    icon_file = 'steamos-logo-light.png'
        except Exception as e:
            print(f"Failed to read Windows theme: {e}")

    icon_path = os.path.join(ICONS_FOLDER, icon_file)
    menu = create_menu(current_mode)
    icon_image = Image.open(icon_path)
    tray_icon = Icon('BigPictureTV', icon=icon_image, menu=menu)
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

        self.constants_path = SETTINGS_FILE
        uic.loadUi(os.path.join(UI_FOLDER, 'design.ui'), self)

        self.setWindowTitle("BigPictureTV - Settings")
        self.setWindowIcon(QIcon(os.path.join(ICONS_FOLDER, 'steamos-logo.png')))
        self.setFixedSize(self.size())

        self.constants = load_constants()
        self.load_settings()

        self.saveButton.clicked.connect(self.save_settings)
        self.systemThemeBox.setChecked(self.constants.get('UseSystemTheme', False))
        self.systemThemeBox.stateChanged.connect(self.toggle_system_theme)

        self.startupCheckBox.setChecked(self.is_startup_shortcut_exist())
        self.startupCheckBox.stateChanged.connect(self.handle_startup_checkbox)

        self.helpButton.clicked.connect(self.open_help_dialog)
        
        self.set_stylesheet()

    def is_startup_shortcut_exist(self):
        startup_dir = winshell.startup()
        shortcut_path = os.path.join(startup_dir, "BigPictureTV.lnk")
        return os.path.exists(shortcut_path)
    
    def handle_startup_checkbox(self, state):
        if state == Qt.Checked:
            self.create_shortcut()
        else:
            self.remove_shortcut()

    def create_shortcut(self):
        startup_dir = winshell.startup()
        shortcut_path = os.path.join(startup_dir, "BigPictureTV.lnk")
        target = os.path.abspath(sys.argv[0])
        icon = target
        winshell.CreateShortcut(
            Path=shortcut_path,
            Target=target,
            Icon=(icon, 0),
            Description="BigPictureTV Startup Shortcut"
        )

    def remove_shortcut(self):
        startup_dir = winshell.startup()
        shortcut_path = os.path.join(startup_dir, "BigPictureTV.lnk")
        if os.path.exists(shortcut_path):
            os.remove(shortcut_path)

    def load_settings(self):
        self.constants = load_constants()
        self.steamEntry.setText(' '.join(self.constants['BIG_PICTURE_KEYWORDS']))
        self.desktopEntry.setText(self.constants['DESKTOP_AUDIO'])
        self.gamemodeEntry.setText(self.constants['GAMEMODE_AUDIO'])

    def save_settings(self):
        self.constants['BIG_PICTURE_KEYWORDS'] = self.steamEntry.text().split()
        self.constants['DESKTOP_AUDIO'] = self.desktopEntry.text()
        self.constants['GAMEMODE_AUDIO'] = self.gamemodeEntry.text()

        try:
            with open(self.constants_path, 'w') as f:
                json.dump(self.constants, f, indent=4)
            QMessageBox.information(self, 'Success', 'Settings saved successfully.')
            restart_main()
        except Exception as e:
            QMessageBox.critical(self, 'Error', f'Failed to save settings: {e}')

    def set_stylesheet(self):
        style_file = os.path.join(os.path.dirname(__file__), UI_FOLDER, 'style.css')
        with open(style_file, 'r') as f:
            self.setStyleSheet(f.read())

    def toggle_system_theme(self, state):
        self.constants['UseSystemTheme'] = (state == Qt.Checked)

    def open_help_dialog(self):
        dialog = HelpDialog(self.styleSheet())
        dialog.exec_()

class HelpDialog(QDialog):
    def __init__(self, stylesheet=None):
        super().__init__()

        self.setWindowTitle("Help")
        uic.loadUi(os.path.join(UI_FOLDER, 'help.ui'), self)

        if stylesheet:
            self.setStyleSheet(stylesheet)

        self.setWindowIcon(QIcon(os.path.join(ICONS_FOLDER, 'steamos-logo.png')))
        self.closeButton.clicked.connect(self.close)
        self.setWindowFlags(self.windowFlags() & ~Qt.WindowContextHelpButtonHint)
        self.setFixedSize(self.size())

def restart_main():
    python = sys.executable
    os.execl(python, python, *sys.argv)

if __name__ == '__main__':
    constants = load_constants()
    current_mode = read_current_mode()

    if current_mode != Mode.DESKTOP:
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
            time.sleep(1)
    except KeyboardInterrupt:
        print("\nExiting")
