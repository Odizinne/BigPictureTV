import sys
import os
import json
import subprocess
import time
import re
import threading
import pygetwindow as gw
from enum import Enum
from PIL import Image
from pystray import Icon, MenuItem, Menu
from PyQt5.QtWidgets import QApplication, QMainWindow, QDialog, QVBoxLayout, QTextEdit, QPushButton, QMessageBox
from PyQt5.QtGui import QIcon
from PyQt5 import uic

class Mode(Enum):
    DESKTOP = 1
    GAMEMODE = 2

tray_icon = None
current_mode = None
constants = None

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
        "DESKTOP_AUDIO": "Headset",
        "UseSystemTheme": False  # Added setting for system theme
    }
    with open(constants_path, 'w') as f:
        json.dump(settings_template, f, indent=4)

    # Function to open settings window
    open_settings_window()

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
    if os.path.exists(file_path):
        return Mode.GAMEMODE
    else:
        return Mode.DESKTOP

def exit_action(icon, item):
    icon.stop()
    os._exit(0)

def create_tray_icon(current_mode):
    global tray_icon

    # Load constants and check for UseSystemTheme setting
    constants = load_constants()
    use_system_theme = constants.get('UseSystemTheme', False)

    # Determine icon path based on system theme
    if use_system_theme:
        # Check Windows theme using winreg
        try:
            import winreg
            reg_path = r'SOFTWARE\Microsoft\Windows\CurrentVersion\Themes\Personalize'
            with winreg.OpenKey(winreg.HKEY_CURRENT_USER, reg_path) as key:
                theme_type = winreg.QueryValueEx(key, 'AppsUseLightTheme')[0]
                if theme_type == 1:
                    icon_path = os.path.join(os.path.dirname(__file__), 'steamos-logo-dark.png')
                else:
                    icon_path = os.path.join(os.path.dirname(__file__), 'steamos-logo-light.png')
        except Exception as e:
            print(f"Failed to read Windows theme: {e}")
            icon_path = os.path.join(os.path.dirname(__file__), 'steamos-logo.png')
    else:
        icon_path = os.path.join(os.path.dirname(__file__), 'steamos-logo.png')

    # Create menu and tray icon
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

        # Determine if the script is bundled or run as a script
        if getattr(sys, 'frozen', False):
            # Bundled by PyInstaller
            basedir = sys._MEIPASS
        else:
            # Run as a script
            basedir = os.path.dirname(__file__)

        self.constants_path = os.path.join(os.environ['APPDATA'], "bigpicture-eternal", 'settings.json')

        # Load the UI
        uic.loadUi(os.path.join(basedir, 'design.ui'), self)

        # Set the window title
        self.setWindowTitle("BigPictureTV - Settings")

        # Set the window icon
        icon_path = os.path.join(basedir, 'steamos-logo.png')  # Path to your icon file
        self.setWindowIcon(QIcon(icon_path))

        # Prevent resizing by setting fixed size
        self.setFixedSize(self.size())

        self.constants = load_constants()
        self.load_settings()

        self.saveButton.clicked.connect(self.save_settings)

        # Apply stylesheet for grey background and widget styling
        self.set_darkmode()

        # Connect systemThemeBox to toggle_system_theme function
        self.systemThemeBox.setChecked(self.constants.get('UseSystemTheme', False))
        self.systemThemeBox.stateChanged.connect(self.toggle_system_theme)

        # Connect helpButton to open_help_dialog function
        self.helpButton.clicked.connect(self.open_help_dialog)

    def load_settings(self):
        self.constants = load_constants()

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

    def set_darkmode(self):
        # Define a stylesheet for rounded corners and other widget styles
        stylesheet = """
            background-color: #171d25;
        }

        QPushButton {
            background-color: #32363d;
            color: white;
            border: 1px solid #303641;
            border-radius: 4px;  /* Rounded corners */
            padding: 5px;
            margin: 8px;
        }

        QLabel {
            color: white;
            margin: 8px;
        }

        QLineEdit {
            background-color: #24282f;
            color: white;
            border: 1px solid #303641;
            border-radius: 4px;  /* Rounded corners */
            padding: 3px;
            margin: 8px;
        }

        QTextEdit {
            background-color: #24282f;
            color: white;
            border: 1px solid #303641;
            border-radius: 4px;  /* Rounded corners */
            padding: 5px;
            margin: 8px;
        }

        QCheckBox {
            margin: 8px;
        }

        """

        # Apply the stylesheet to the window
        self.setStyleSheet(stylesheet)

    def toggle_system_theme(self, state):
        if state == 2:  # Qt.Checked
            self.constants['UseSystemTheme'] = True
        else:
            self.constants['UseSystemTheme'] = False

    def open_help_dialog(self):
        dialog = HelpDialog(self.styleSheet())  # Pass main window's stylesheet to dialog
        dialog.exec_()

class HelpDialog(QDialog):
    def __init__(self, stylesheet=None):
        super().__init__()

        self.setWindowTitle("Help")
        self.setWindowIcon(QIcon("icon_path"))  # Set the icon path as per your requirement

        layout = QVBoxLayout()

        help_text = QTextEdit()
        help_text.setHtml("""
            <b>- Steam Big Picture Keywords:</b><br><br>
            This is used to search for Big Picture window.<br>
            Adjust depending on your Steam language.<br><br>
            
            <b>- DESKTOP / GAMEMODE audio device:</b><br><br>
            The devices that will be used when switching modes.<br>
            You can specify keywords here.<br>
            If your device full name is "Headset Earphone (CORSAIR VOID ELITE)",<br>
            you can specify "Corsair Headset" and it will be valid.
        """)
        help_text.setReadOnly(True)

        layout.addWidget(help_text)

        close_button = QPushButton("Close")
        close_button.clicked.connect(self.close)

        layout.addWidget(close_button)

        self.setLayout(layout)

        if stylesheet:
            self.setStyleSheet(stylesheet)  # Apply main window's stylesheet to dialog

        self.setFixedSize(400, 265)

def restart_main():
    python = sys.executable
    os.execl(python, python, *sys.argv)

if __name__ == '__main__':
    GAMEMODE_SCREEN = "/external"
    DESKTOP_SCREEN = "/internal"
    constants = load_constants()
    current_mode = read_current_mode()
    SLEEP_TIME = 1

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
            time.sleep(SLEEP_TIME)
    except KeyboardInterrupt:
        print("\nExiting")
