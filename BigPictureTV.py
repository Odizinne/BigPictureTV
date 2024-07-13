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
        self.setWindowTitle('BigPictureTV')
        self.setWindowIcon(QIcon(os.path.join(ICONS_FOLDER, 'steamos-logo.png')))
        self.tray_icon = None
        self.current_mode = None
        self.gamemode_audio = None
        self.desktop_audio = None
        self.bigpicture_keywords = None
        self.check_rate = None
        self.first_run = False
        self.paused = False
        self.timer = QTimer()
        self.settings_window = None
        self.gamemode_screen = "/external"
        self.desktop_screen = "/internal"
        self.connect_ui()
        self.load_settings()
        self.is_audio_device_cmdlets_installed()
        self.current_mode = self.read_current_mode()

        if self.current_mode != Mode.DESKTOP:
            self.switch_mode(Mode.DESKTOP)

        self.tray_icon = self.create_tray_icon()

        self.timer.timeout.connect(self.update_mode)
        self.update_mode_timer_interval(self.check_rate)

        if self.first_run:
            self.settings_window.show()
            self.first_run = False

    def connect_ui(self):
        self.ui.disableAudioCheckbox.stateChanged.connect(self.on_disableAudioCheckbox_stateChanged)
        self.ui.startupCheckBox.stateChanged.connect(self.on_startupCheckBox_stateChanged)
        self.ui.steamEntry.textChanged.connect(self.on_steamEntry_textChanged)
        self.ui.gamemodeEntry.textChanged.connect(self.on_gamemodeEntry_textChanged)
        self.ui.desktopEntry.textChanged.connect(self.on_desktopEntry_textChanged)
        self.ui.checkRateSpinBox.valueChanged.connect(self.on_checkRateSpinBox_valueChanged)
        self.ui.helpButton.clicked.connect(self.show_help)
        self.ui.startupCheckBox.setChecked(self.check_startup_shortcut())

    def on_disableAudioCheckbox_stateChanged(self, state):
        self.ui.desktopEntry.setEnabled(not state)
        self.ui.desktopLabel.setEnabled(not state)
        self.ui.gamemodeEntry.setEnabled(not state)
        self.ui.gamemodeLabel.setEnabled(not state)
        self.save_settings()

    def on_startupCheckBox_stateChanged(self, state):
        self.manage_startup_shortcut(state)

    def on_steamEntry_textChanged(self, text):
        self.bigpicture_keywords = text.split()
        self.save_settings()

    def on_gamemodeEntry_textChanged(self, text):
        self.gamemode_audio = text
        self.save_settings()

    def on_desktopEntry_textChanged(self, text):
        self.desktop_audio = text
        self.save_settings()

    def on_checkRateSpinBox_valueChanged(self, value):
        self.check_rate = value
        self.save_settings()
        self.update_mode_timer_interval(value)

    def show_help(self):
        help_dialog = HelpDialog()
        help_dialog.exec()

    def create_default_settings(self):
        settings_template = {
            "BIG_PICTURE_KEYWORDS": ["Steam", "mode", "Big", "Picture"],
            "GAMEMODE_AUDIO": "TV",
            "DESKTOP_AUDIO": "Headset",
            "DisableAudioSwitch": False,
            "CheckRate": 1000
        }
        with open(SETTINGS_FILE, 'w') as f:
            json.dump(settings_template, f, indent=4)
        self.first_run = True

    def load_settings(self):
        if not os.path.exists(os.path.dirname(SETTINGS_FILE)):
            os.makedirs(os.path.dirname(SETTINGS_FILE))

        if not os.path.exists(SETTINGS_FILE):
            self.create_default_settings()

        with open(SETTINGS_FILE, 'r') as f:
            settings = json.load(f)
        
        self.ui.steamEntry.setText(' '.join(settings.get('BIG_PICTURE_KEYWORDS', [])))
        self.bigpicture_keywords = settings.get('BIG_PICTURE_KEYWORDS', [])
        self.ui.gamemodeEntry.setText(settings.get('GAMEMODE_AUDIO', ''))
        self.gamemode_audio = settings.get('GAMEMODE_AUDIO', '')
        self.ui.desktopEntry.setText(settings.get('DESKTOP_AUDIO', ''))
        self.desktop_audio = settings.get('DESKTOP_AUDIO', '')
        self.ui.disableAudioCheckbox.setChecked(settings.get('DisableAudioSwitch', False))
        self.ui.checkRateSpinBox.setValue(settings.get('CheckRate', 1000))
        self.check_rate = settings.get('CheckRate', 1000)

        self.ui.desktopEntry.setEnabled(not self.ui.disableAudioCheckbox.isChecked())
        self.ui.desktopLabel.setEnabled(not self.ui.disableAudioCheckbox.isChecked())
        self.ui.gamemodeEntry.setEnabled(not self.ui.disableAudioCheckbox.isChecked())
        self.ui.gamemodeLabel.setEnabled(not self.ui.disableAudioCheckbox.isChecked())

    def save_settings(self):
        settings = {
            "BIG_PICTURE_KEYWORDS": self.ui.steamEntry.text().split(),
            "GAMEMODE_AUDIO": self.ui.gamemodeEntry.text(),
            "DESKTOP_AUDIO": self.ui.desktopEntry.text(),
            "DisableAudioSwitch": self.ui.disableAudioCheckbox.isChecked(),
            "CheckRate": self.ui.checkRateSpinBox.value()
        }

        with open(SETTINGS_FILE, 'w') as f:
            json.dump(settings, f, indent=4)

    def read_current_mode(self):
        file_path = self.get_mode_file_path()
        return Mode.GAMEMODE if os.path.exists(file_path) else Mode.DESKTOP

    def get_mode_file_path(self):
        app_data_folder = os.path.join(os.environ['APPDATA'], 'BigPictureTV')
        if not os.path.exists(app_data_folder):
            os.makedirs(app_data_folder)
        return os.path.join(app_data_folder, 'current_mode.txt')

    def switch_mode(self, mode):
        disabled_audio = self.ui.disableAudioCheckbox.isChecked()

        if mode == Mode.GAMEMODE:
            self.switch_screen(self.gamemode_screen)
            if not disabled_audio:
                self.switch_audio(self.gamemode_audio)
            self.current_mode = Mode.GAMEMODE
        else:
            self.switch_screen(self.desktop_screen)
            if not disabled_audio:
                self.switch_audio(self.desktop_audio)
            self.current_mode = Mode.DESKTOP

        self.write_current_mode(self.current_mode)

        if self.tray_icon:
            self.update_tray_icon_menu()

    def switch_screen(self, mode):
        subprocess.run(["DisplaySwitch.exe", str(mode)])

    def is_audio_device_cmdlets_installed(self):
        cmd = 'powershell "Get-Module -ListAvailable -Name AudioDeviceCmdlets"'
        result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
        if "AudioDeviceCmdlets" in result.stdout:
            return True
        else:
            self.ui.disableAudioCheckbox.setChecked(True)
            self.ui.disableAudioCheckbox.setEnabled(False)
            self.ui.desktopEntry.setEnabled(False)
            self.ui.desktopLabel.setEnabled(False)
            self.ui.gamemodeEntry.setEnabled(False)
            self.ui.gamemodeLabel.setEnabled(False)
            return False

    def get_audio_devices(self):
        cmd = "powershell Get-AudioDevice -list"
        output = subprocess.check_output(cmd, shell=True, text=True)
        devices = re.findall(r'Index\s+:\s+(\d+)\s+.*?Name\s+:\s+(.*?)\s+ID\s+:\s+{(.*?)}', output, re.DOTALL)
        return devices
    
    def set_audio_device(self, device_name, devices):
        device_words = device_name.lower().split()

        for index, name, _ in devices:
            lower_name = name.lower()
            if all(word in lower_name for word in device_words):
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
        print('Checking mode...')
        if self.is_bigpicture_running() and self.current_mode != Mode.GAMEMODE and not self.read_stream_status():
            self.switch_mode(Mode.GAMEMODE)
        elif not self.is_bigpicture_running() and self.current_mode != Mode.DESKTOP:
            self.switch_mode(Mode.DESKTOP)

    def update_mode_timer_interval(self, check_rate):
        timer_interval = check_rate
        self.timer.setInterval(timer_interval)
        self.timer.start()

    def create_tray_icon(self):
        tray_icon = QSystemTrayIcon(QIcon(os.path.join(ICONS_FOLDER, 'steamos-logo.png')))
        tray_icon.setToolTip('BigPictureTV')
        menu = self.create_menu()
        tray_icon.setContextMenu(menu)
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

        if self.paused:
            pause_resume_action = QAction('Resume detection', menu)
        else:
            pause_resume_action = QAction('Pause detection', menu)
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
        return any(all(word in window_title for word in self.bigpicture_keywords)
                   for window_title in gw.getAllTitles())

    def write_current_mode(self, current_mode):
        file_path = self.get_mode_file_path()
        if current_mode == Mode.GAMEMODE:
            with open(file_path, 'w'):
                pass
        elif current_mode == Mode.DESKTOP:
            if os.path.exists(file_path):
                os.remove(file_path)

    def toggle_detection(self):
        self.paused = not self.paused

        if self.paused:
            self.timer.stop()
            self.tray_icon.setToolTip('BigPictureTV (Paused)')
            self.update_tray_icon_menu()
        else:
            self.timer.start()
            self.tray_icon.setToolTip('BigPictureTV')
            self.update_tray_icon_menu()

    def closeEvent(self, event):
        event.ignore()
        self.hide()

    def manage_startup_shortcut(self, state):
        if state:
            print('Creating startup shortcut...')
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

        else: 
            startup_folder = winshell.startup()
            shortcut_path = os.path.join(startup_folder, 'BigPictureTV.lnk')
            if os.path.exists(shortcut_path):
                os.remove(shortcut_path)

    def check_startup_shortcut(self):
        startup_folder = winshell.startup()
        shortcut_path = os.path.join(startup_folder, 'BigPictureTV.lnk')
        return os.path.exists(shortcut_path)
    
class HelpDialog(QDialog):
    def __init__(self):
        super().__init__()
        self.ui = Ui_Dialog()
        self.ui.setupUi(self)
        self.ui.closeButton.clicked.connect(self.close)

if __name__ == '__main__':
    shared_memory = QSharedMemory('BigPictureTVSharedMemory')
    if shared_memory.attach() or not shared_memory.create(1):
        sys.exit(0)

    app = QApplication(sys.argv)
    big_picture_tv = BigPictureTV()
    sys.exit(app.exec())
