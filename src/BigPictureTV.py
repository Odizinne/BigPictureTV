import sys
import os
import json
import subprocess
import time
import re
import winshell
import pygetwindow as gw
from enum import Enum
from PyQt5.QtWidgets import QApplication, QMainWindow, QDialog, QMessageBox, QSystemTrayIcon, QMenu, QAction
from PyQt5.QtGui import QIcon, QCursor
from PyQt5.QtCore import Qt, QTimer, QPoint, pyqtSignal, QSharedMemory
from PyQt5 import uic

class Mode(Enum):
    DESKTOP = 1
    GAMEMODE = 2

if getattr(sys, 'frozen', False):
    ICONS_FOLDER = 'icons'
    UI_FOLDER = 'ui'
    print("Running in frozen mode (cx_Freeze)")
else:
    UI_FOLDER = os.path.join(os.path.dirname(__file__), 'ui')
    ICONS_FOLDER = os.path.join(os.path.dirname(__file__), 'icons')
    print("Running in normal Python mode")

GAMEMODE_SCREEN = "/external"
DESKTOP_SCREEN = "/internal"
SETTINGS_FILE = os.path.join(os.environ['APPDATA'], "BigPictureTV", "settings.json")

tray_icon = None
current_mode = None
constants = None
settings_window = None
first_run = False

def load_constants():
    if not os.path.exists(os.path.dirname(SETTINGS_FILE)):
        os.makedirs(os.path.dirname(SETTINGS_FILE))

    if not os.path.exists(SETTINGS_FILE):
        create_default_settings()

    with open(SETTINGS_FILE, 'r') as f:
        return json.load(f)

def create_default_settings():
    global first_run
    settings_template = {
        "BIG_PICTURE_KEYWORDS": ["Steam", "mode", "Big", "Picture"],
        "GAMEMODE_AUDIO": "TV",
        "DESKTOP_AUDIO": "Headset",
        "DisableAudioSwitch": False,
        "CheckRate": 1000
    }
    with open(SETTINGS_FILE, 'w') as f:
        json.dump(settings_template, f, indent=4)
    first_run = True

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
    global constants
    constants = load_constants()
    disabled_audio = constants.get('DisableAudioSwitch')
    installed_audio = is_audio_device_cmdlets_installed()

    if mode == Mode.GAMEMODE:
        switch_screen(GAMEMODE_SCREEN)
        if not disabled_audio and installed_audio:
            switch_audio(constants['GAMEMODE_AUDIO'])
        current_mode = Mode.GAMEMODE
    else:
        switch_screen(DESKTOP_SCREEN)
        if not disabled_audio and installed_audio:
            switch_audio(constants['DESKTOP_AUDIO'])
        current_mode = Mode.DESKTOP

    write_current_mode(current_mode)

    if tray_icon:
        update_tray_icon_menu()

def create_menu(current_mode):
    menu = QMenu()
    
    mode_action = QAction(f'Mode: {current_mode.name}', menu)
    mode_action.setDisabled(True)
    menu.addAction(mode_action)

    menu.addSeparator()

    settings_action = QAction('Settings', menu)
    settings_action.triggered.connect(open_settings_window)
    menu.addAction(settings_action)

    exit_action = QAction('Exit', menu)
    exit_action.triggered.connect(exit_application)
    menu.addAction(exit_action)

    return menu

def update_tray_icon_menu():
    global tray_icon
    if tray_icon:
        tray_icon.setContextMenu(create_menu(current_mode))

def is_bigpicture_running():
    return any(all(word in window_title for word in constants['BIG_PICTURE_KEYWORDS'])
               for window_title in gw.getAllTitles())

def get_mode_file_path():
    app_data_folder = os.path.join(os.environ['APPDATA'], 'BigPictureTV')
    if not os.path.exists(app_data_folder):
        os.makedirs(app_data_folder)
    return os.path.join(app_data_folder, 'current_mode.txt')

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

def exit_application():
    QApplication.quit()

def create_tray_icon(current_mode):
    tray_icon = QSystemTrayIcon(QIcon(os.path.join(ICONS_FOLDER, 'steamos-logo.png')))
    tray_icon.setToolTip('BigPictureTV')

    menu = create_menu(current_mode)
    tray_icon.setContextMenu(menu)
    tray_icon.show()

    def on_tray_icon_activated(reason):
        if reason == QSystemTrayIcon.Context:
            cursor_pos = QCursor.pos()
            screen_geometry = QApplication.desktop().screenGeometry()
            menu_height = menu.sizeHint().height()
            adjusted_pos = cursor_pos - QPoint(0, menu_height)

            if adjusted_pos.y() < screen_geometry.top():
                adjusted_pos.setY(screen_geometry.top())

            menu.exec_(adjusted_pos)

    tray_icon.activated.connect(on_tray_icon_activated)

    return tray_icon

def open_settings_window():
    global settings_window

    if settings_window is None:
        settings_window = SettingsWindow()
    if not settings_window.isVisible():
        settings_window.show()
    settings_window.raise_()
    settings_window.activateWindow()

def is_audio_device_cmdlets_installed():
    try:
        cmd = 'powershell "Get-Module -ListAvailable -Name AudioDeviceCmdlets"'
        result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
        if "AudioDeviceCmdlets" in result.stdout:
            return True
        return False
    except Exception as e:
        print(f"Error checking AudioDeviceCmdlets installation: {e}")
        return False

class SettingsWindow(QMainWindow):
    checkRateChanged = pyqtSignal(int)
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

        self.startupCheckBox.setChecked(self.is_startup_shortcut_exist())
        self.startupCheckBox.stateChanged.connect(self.handle_startup_checkbox)

        self.disableAudioCheckbox.stateChanged.connect(self.toggle_audio_fields)
        self.helpButton.clicked.connect(self.open_help_dialog)
        self.audioInstallButton.clicked.connect(self.install_audio_device_cmdlets)

        self.checkRateSpinBox.valueChanged.connect(self.update_check_rate)
        check_rate = self.constants.get('CheckRate', 1000)
        self.checkRateSpinBox.setValue(check_rate)

        if is_audio_device_cmdlets_installed():
            self.audioInstallButton.setText("AudioDeviceCmdlets Installed")
            self.audioInstallButton.setEnabled(False)
        else:
            self.disableAudioCheckbox.setEnabled(False)
            self.desktopEntry.setEnabled(False)
            self.gamemodeEntry.setEnabled(False)

        disable_audio = self.constants.get('DisableAudioSwitch', False)
        self.disableAudioCheckbox.setChecked(disable_audio)

    def update_check_rate(self, value):
        self.constants['CheckRate'] = value
        self.checkRateChanged.emit(value)

    def closeEvent(self, event):
        event.ignore()
        self.hide()

    def toggle_audio_fields(self, state):
        disable_audio = state == Qt.Checked
        self.gamemodeEntry.setEnabled(not disable_audio)
        self.desktopEntry.setEnabled(not disable_audio)

    def load_settings(self):
        self.constants = load_constants()
        self.steamEntry.setText(' '.join(self.constants['BIG_PICTURE_KEYWORDS']))
        self.desktopEntry.setText(self.constants['DESKTOP_AUDIO'])
        self.gamemodeEntry.setText(self.constants['GAMEMODE_AUDIO'])
        
        disable_audio = self.constants.get('DisableAudioSwitch', False)
        self.disableAudioCheckbox.setChecked(disable_audio)

        check_rate = self.constants.get('CheckRate', 1000)
        self.checkRateSpinBox.setValue(check_rate)

        self.desktopEntry.setEnabled(not disable_audio)
        self.gamemodeEntry.setEnabled(not disable_audio)

    def save_settings(self):
        self.constants['BIG_PICTURE_KEYWORDS'] = self.steamEntry.text().split()
        self.constants['DESKTOP_AUDIO'] = self.desktopEntry.text()
        self.constants['GAMEMODE_AUDIO'] = self.gamemodeEntry.text()
        self.constants['DisableAudioSwitch'] = self.disableAudioCheckbox.isChecked()
        self.constants['CheckRate'] = self.checkRateSpinBox.value()

        try:
            with open(self.constants_path, 'w') as f:
                json.dump(self.constants, f, indent=4)
            QMessageBox.information(self, 'Success', 'Settings saved successfully.')
            self.hide()
        except Exception as e:
            QMessageBox.critical(self, 'Error', f'Failed to save settings: {e}')

    def open_help_dialog(self):
        dialog = HelpDialog(self.styleSheet())
        dialog.exec_()

    def handle_startup_checkbox(self, state):
        if state == Qt.Checked:
            self.create_startup_shortcut()
        else:
            self.remove_startup_shortcut()

    def create_startup_shortcut(self):
        target_path = os.path.join(os.getcwd(), 'BigPictureTV.exe')
        startup_folder = winshell.startup()
        shortcut_path = os.path.join(startup_folder, 'BigPictureTV.lnk')
        working_directory = os.path.dirname(target_path)

        winshell.CreateShortcut(
            Path=shortcut_path,
            Target=target_path,
            Icon=(target_path, 0),
            Description="Launch BigPictureTV",
            StartIn=working_directory
        )

    def remove_startup_shortcut(self):
        startup_folder = winshell.startup()
        shortcut_path = os.path.join(startup_folder, 'BigPictureTV.lnk')
        if os.path.exists(shortcut_path):
            os.remove(shortcut_path)

    def is_startup_shortcut_exist(self):
        startup_folder = winshell.startup()
        shortcut_path = os.path.join(startup_folder, 'BigPictureTV.lnk')
        return os.path.exists(shortcut_path)
 
    def install_audio_device_cmdlets(self):
        try:
            self.audioInstallButton.setText("Installing...")
            self.audioInstallButton.setEnabled(False)
            cmd = 'powershell Install-Module -Name AudioDeviceCmdlets -Scope CurrentUser -Force -AllowClobber'
            result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
            if result.returncode == 0:
                QMessageBox.information(self, 'Success', 'AudioDeviceCmdlets installed successfully.')
                self.audioInstallButton.setText("AudioDeviceCmdlets Installed")
                self.audioInstallButton.setEnabled(False)
                self.disableAudioCheckbox.setEnabled(True)
                self.desktopEntry.setEnabled(True)
                self.gamemodeEntry.setEnabled(True)
            else:
                QMessageBox.critical(self, 'Error', 'Failed to install AudioDeviceCmdlets. See the console for more details.')
                print(result.stderr)
                self.audioInstallButton.setEnabled(True)
                self.audioInstallButton.setText("Install AudioDeviceCmdlets")
        except Exception as e:
            QMessageBox.critical(self, 'Error', f'Failed to install AudioDeviceCmdlets: {e}')
            self.audioInstallButton.setEnabled(True)
            self.audioInstallButton.setText("Install AudioDeviceCmdlets")

class HelpDialog(QDialog):
    def __init__(self, stylesheet=None):
        super().__init__()

        uic.loadUi(os.path.join(UI_FOLDER, 'help.ui'), self)
        self.setWindowTitle("Help")
        if stylesheet:
            self.setStyleSheet(stylesheet)

        self.setWindowIcon(QIcon(os.path.join(ICONS_FOLDER, 'steamos-logo.png')))
        self.closeButton.clicked.connect(self.close)
        self.setWindowFlags(self.windowFlags() & ~Qt.WindowContextHelpButtonHint)
        self.setFixedSize(self.size())

if __name__ == '__main__':
    shared_memory = QSharedMemory('BigPictureTVSharedMemory')

    if shared_memory.attach() or not shared_memory.create(1):
        sys.exit(0)

    app = QApplication(sys.argv)
    current_mode = read_current_mode()
    constants = load_constants()

    style_file = os.path.join(UI_FOLDER, 'style.qss')
    if os.path.exists(style_file):
        with open(style_file, 'r') as f:
            app.setStyleSheet(f.read())

    if current_mode != Mode.DESKTOP:
        switch_mode(Mode.DESKTOP)

    tray_icon = create_tray_icon(current_mode)

    timer = QTimer()

    def update_mode_timer_interval(check_rate):
        timer_interval = check_rate
        timer.setInterval(timer_interval)
        timer.start()

    settings_window = SettingsWindow()
    settings_window.checkRateChanged.connect(update_mode_timer_interval)

    timer.timeout.connect(lambda: (
        switch_mode(Mode.GAMEMODE) if is_bigpicture_running() and current_mode != Mode.GAMEMODE and not read_stream_status() else None,
        switch_mode(Mode.DESKTOP) if not is_bigpicture_running() and current_mode != Mode.DESKTOP else None
    ))

    initial_check_rate = constants.get('CheckRate', 1000)
    update_mode_timer_interval(initial_check_rate)

    if first_run:
        settings_window.show()
        first_run = False
    
    sys.exit(app.exec_())