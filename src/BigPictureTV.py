import sys
import os
import json
import subprocess
import time
import re
import winshell
import pygetwindow as gw
from enum import Enum
from PyQt6.QtWidgets import QApplication, QSystemTrayIcon, QMenu, QMainWindow, QDialog
from PyQt6.QtGui import QIcon, QAction
from PyQt6.QtCore import QTimer, QSharedMemory
from design import Ui_MainWindow
from help import Ui_Dialog

SETTINGS_FILE = os.path.join(os.environ['APPDATA'], "BigPictureTV", "settings.json")
ICONS_FOLDER = 'icons' if getattr(sys, 'frozen', False) else os.path.join(os.path.dirname(__file__), 'icons')

class Mode(Enum):
    DESKTOP = 1
    GAMEMODE = 2

class BigPictureTV(QMainWindow):
    def __init__(self):
        super().__init__()
        self.ui = Ui_MainWindow()
        self.ui.setupUi(self)
        self.setWindowTitle('BigPictureTV - Settings')
        self.setWindowIcon(QIcon(os.path.join(ICONS_FOLDER, 'steamos-logo.png')))
        self.tray_icon = None
        self.current_mode = None
        self.settings = {}
        self.first_run = False
        self.paused = False
        self.timer = QTimer()
        self.gamemode_screen = "/external"
        self.desktop_screen = "/internal"
        self.load_settings()
        self.initialize_ui()
        self.is_audio_device_cmdlets_installed()
        self.current_mode = self.read_current_mode()
        self.switch_mode(self.current_mode or Mode.DESKTOP)
        self.tray_icon = self.create_tray_icon()
        self.timer.timeout.connect(self.update_mode)
        self.update_mode_timer_interval(self.settings.get("CheckRate", 1000))
        if self.first_run:
            self.show()
            self.first_run = False

    def initialize_ui(self):
        self.ui.disableAudioCheckbox.stateChanged.connect(self.on_disableAudioCheckbox_stateChanged)
        self.ui.startupCheckBox.stateChanged.connect(self.on_startupCheckBox_stateChanged)
        self.ui.steamEntry.textChanged.connect(self.on_settings_changed)
        self.ui.gamemodeEntry.textChanged.connect(self.on_settings_changed)
        self.ui.desktopEntry.textChanged.connect(self.on_settings_changed)
        self.ui.checkRateSpinBox.valueChanged.connect(self.on_settings_changed)
        self.ui.helpButton.clicked.connect(self.show_help)
        self.ui.startupCheckBox.setChecked(self.check_startup_shortcut())

    def on_disableAudioCheckbox_stateChanged(self, state):
        self.toggle_audio_settings(not state)
        self.save_settings()

    def on_startupCheckBox_stateChanged(self, state):
        self.manage_startup_shortcut(state)

    def on_settings_changed(self):
        self.save_settings()

    def toggle_audio_settings(self, enabled):
        self.ui.desktopEntry.setEnabled(enabled)
        self.ui.desktopLabel.setEnabled(enabled)
        self.ui.gamemodeEntry.setEnabled(enabled)
        self.ui.gamemodeLabel.setEnabled(enabled)

    def create_default_settings(self):
        self.settings = {
            "BIG_PICTURE_KEYWORDS": ["Steam", "mode", "Big", "Picture"],
            "GAMEMODE_AUDIO": "TV",
            "DESKTOP_AUDIO": "Headset",
            "DisableAudioSwitch": False,
            "CheckRate": 1000
        }
        self.apply_settings()
        self.save_settings()
        self.first_run = True

    def load_settings(self):
        if not os.path.exists(os.path.dirname(SETTINGS_FILE)):
            os.makedirs(os.path.dirname(SETTINGS_FILE))
        if not os.path.exists(SETTINGS_FILE):
            self.create_default_settings()
        else:
            with open(SETTINGS_FILE, 'r') as f:
                self.settings = json.load(f)
        self.apply_settings()

    def apply_settings(self):
        self.ui.steamEntry.setText(' '.join(self.settings.get('BIG_PICTURE_KEYWORDS', [])))
        self.ui.gamemodeEntry.setText(self.settings.get('GAMEMODE_AUDIO', ''))
        self.ui.desktopEntry.setText(self.settings.get('DESKTOP_AUDIO', ''))
        self.ui.disableAudioCheckbox.setChecked(self.settings.get('DisableAudioSwitch', False))
        self.ui.checkRateSpinBox.setValue(self.settings.get('CheckRate', 1000))
        self.toggle_audio_settings(not self.ui.disableAudioCheckbox.isChecked())

    def save_settings(self):
        self.settings = {
            "BIG_PICTURE_KEYWORDS": self.ui.steamEntry.text().split(),
            "GAMEMODE_AUDIO": self.ui.gamemodeEntry.text(),
            "DESKTOP_AUDIO": self.ui.desktopEntry.text(),
            "DisableAudioSwitch": self.ui.disableAudioCheckbox.isChecked(),
            "CheckRate": self.ui.checkRateSpinBox.value()
        }
        os.makedirs(os.path.dirname(SETTINGS_FILE), exist_ok=True)
        with open(SETTINGS_FILE, 'w') as f:
            json.dump(self.settings, f, indent=4)

    def read_current_mode(self):
        return Mode.GAMEMODE if os.path.exists(self.get_mode_file_path()) else Mode.DESKTOP

    def get_mode_file_path(self):
        app_data_folder = os.path.join(os.environ['APPDATA'], 'BigPictureTV')
        if not os.path.exists(app_data_folder):
            os.makedirs(app_data_folder)
        return os.path.join(app_data_folder, 'current_mode.txt')
    
    def switch_mode(self, mode):
        if mode == self.current_mode:
            return
        self.current_mode = mode
        self.switch_screen(self.gamemode_screen if mode == Mode.GAMEMODE else self.desktop_screen)
        if not self.ui.disableAudioCheckbox.isChecked():
            self.switch_audio(self.settings.get('GAMEMODE_AUDIO') if mode == Mode.GAMEMODE else self.settings.get('DESKTOP_AUDIO'))
        self.write_current_mode(mode)
        if self.tray_icon:
            self.update_tray_icon_menu()

    def switch_screen(self, mode):
        subprocess.run(["DisplaySwitch.exe", str(mode)])

    def is_audio_device_cmdlets_installed(self):
        cmd = 'powershell "Get-Module -ListAvailable -Name AudioDeviceCmdlets"'
        result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
        if "AudioDeviceCmdlets" in result.stdout:
            return True
        self.ui.disableAudioCheckbox.setChecked(True)
        self.ui.disableAudioCheckbox.setEnabled(False)
        self.toggle_audio_settings(False)
        return False

    def get_audio_devices(self):
        cmd = "powershell Get-AudioDevice -list"
        output = subprocess.check_output(cmd, shell=True, text=True)
        devices = re.findall(r'Index\s+:\s+(\d+)\s+.*?Name\s+:\s+(.*?)\s+ID\s+:\s+{(.*?)}', output, re.DOTALL)
        return devices

    def set_audio_device(self, device_name, devices):
        device_words = device_name.lower().split()
        for index, name, _ in devices:
            if all(word in name.lower() for word in device_words):
                cmd = f"powershell set-audiodevice -index {index}"
                result = subprocess.run(cmd, shell=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                return result.returncode == 0
        return False

    def switch_audio(self, audio_output):
        devices = self.get_audio_devices()
        success = self.set_audio_device(audio_output, devices)
        retries = 0
        while not success and retries < 10:
            print("Failed to switch audio, retrying...")
            time.sleep(1)
            success = self.set_audio_device(audio_output, devices)
            retries += 1
        if not success:
            print("Failed to switch audio after 10 attempts.")

    def read_stream_status(self):
        file_path = os.path.join(os.environ['APPDATA'], "sunshine-status", "status.txt")
        return os.path.exists(file_path)
    
    def update_mode(self):
        if self.is_bigpicture_running() and self.current_mode != Mode.GAMEMODE and not self.read_stream_status():
            self.switch_mode(Mode.GAMEMODE)
        elif not self.is_bigpicture_running() and self.current_mode != Mode.DESKTOP:
            self.switch_mode(Mode.DESKTOP)

    def update_mode_timer_interval(self, check_rate):
        self.timer.setInterval(check_rate)
        self.timer.start()

    def create_tray_icon(self):
        tray_icon = QSystemTrayIcon(QIcon(os.path.join(ICONS_FOLDER, 'steamos-logo.png')))
        tray_icon.setToolTip('BigPictureTV')
        tray_icon.setContextMenu(self.create_menu())
        tray_icon.show()
        return tray_icon

    def create_menu(self):
        menu = QMenu()
        mode_action = QAction(f'Current mode: {self.current_mode.name}', menu)
        mode_action.setEnabled(False)
        menu.addAction(mode_action)
        tray_state_action = QAction(f'Detection state: {"Paused" if self.paused else "Active"}', menu)
        tray_state_action.setEnabled(False)
        menu.addAction(tray_state_action)
        menu.addSeparator()
        pause_resume_action = QAction('Resume detection' if self.paused else 'Pause detection', menu)
        pause_resume_action.triggered.connect(self.toggle_detection)
        menu.addAction(pause_resume_action)
        settings_action = QAction('Settings', menu)
        settings_action.triggered.connect(self.show)
        menu.addAction(settings_action)
        exit_action = QAction('Exit', menu)
        exit_action.triggered.connect(QApplication.quit)
        menu.addAction(exit_action)
        return menu

    def update_tray_icon_menu(self):
        if self.tray_icon:
            self.tray_icon.setContextMenu(self.create_menu())

    def is_bigpicture_running(self):
        return any(all(word in window_title for word in self.settings.get('BIG_PICTURE_KEYWORDS', []))
                   for window_title in gw.getAllTitles())

    def write_current_mode(self, current_mode):
        file_path = self.get_mode_file_path()
        if current_mode == Mode.GAMEMODE:
            open(file_path, 'w').close()
        elif current_mode == Mode.DESKTOP and os.path.exists(file_path):
            os.remove(file_path)

    def toggle_detection(self):
        self.paused = not self.paused
        if self.paused:
            self.timer.stop()
            self.tray_icon.setToolTip('BigPictureTV (Paused)')
        else:
            self.timer.start()
            self.tray_icon.setToolTip('BigPictureTV')
        self.update_tray_icon_menu()

    def closeEvent(self, event):
        event.ignore()
        self.hide()

    def manage_startup_shortcut(self, state):
        target_path = os.path.join(os.getcwd(), 'BigPictureTV.exe')
        startup_folder = winshell.startup()
        shortcut_path = os.path.join(startup_folder, 'BigPictureTV.lnk')
        if state:
            winshell.CreateShortcut(
                Path=shortcut_path,
                Target=target_path,
                Icon=(target_path, 0),
                Description="Launch BigPictureTV",
                StartIn=os.path.dirname(target_path)
            )
        elif os.path.exists(shortcut_path):
            os.remove(shortcut_path)

    def check_startup_shortcut(self):
        return os.path.exists(os.path.join(winshell.startup(), 'BigPictureTV.lnk'))

    def show_help(self):
        help_dialog = HelpDialog()
        help_dialog.exec()

class HelpDialog(QDialog):
    def __init__(self):
        super().__init__()
        self.ui = Ui_Dialog()
        self.ui.setupUi(self)
        self.setWindowTitle('BigPictureTV - Help')
        self.setWindowIcon(QIcon(os.path.join(ICONS_FOLDER, 'steamos-logo.png')))
        self.ui.closeButton.clicked.connect(self.close)

if __name__ == '__main__':
    shared_memory = QSharedMemory('BigPictureTVSharedMemory')
    if shared_memory.attach() or not shared_memory.create(1):
        sys.exit(0)
    app = QApplication(sys.argv)
    big_picture_tv = BigPictureTV()
    sys.exit(app.exec())
