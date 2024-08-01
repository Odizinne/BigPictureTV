import sys
import os
import json
import subprocess
import time
import re
import winshell
import configparser
import pygetwindow as gw
from enum import Enum
import darkdetect
from PyQt6.QtWidgets import QApplication, QSystemTrayIcon, QMenu, QMainWindow
from PyQt6.QtGui import QIcon, QAction
from PyQt6.QtCore import QTimer, QSharedMemory
from design import Ui_MainWindow
from steam_language_reader import get_big_picture_window_title

MONITOR_CONFIG_FILE = os.path.join(os.environ["APPDATA"], "BigPictureTV", "desktop_monitor_config.cfg")
SETTINGS_FILE = os.path.join(os.environ["APPDATA"], "BigPictureTV", "settings.json")
ICONS_FOLDER = "icons" if getattr(sys, "frozen", False) else os.path.join(os.path.dirname(__file__), "icons")
MULTIMONITORTOOL_PATH = "dependencies/MultiMonitorTool.exe"


class Mode(Enum):
    DESKTOP = 1
    GAMEMODE = 2


def get_mode_file_path():
    app_data_folder = os.path.join(os.environ["APPDATA"], "BigPictureTV")
    if not os.path.exists(app_data_folder):
        os.makedirs(app_data_folder)
    return os.path.join(app_data_folder, "current_mode.txt")


def read_current_mode():
    return Mode.GAMEMODE if os.path.exists(get_mode_file_path()) else Mode.DESKTOP


def get_audio_devices():
    cmd = "powershell Get-AudioDevice -list"
    output = subprocess.check_output(cmd, shell=True, text=True)
    devices = re.findall(r"Index\s+:\s+(\d+)\s+.*?Name\s+:\s+(.*?)\s+ID\s+:\s+{(.*?)}", output, re.DOTALL)
    return devices


def set_audio_device(device_name, devices):
    device_words = device_name.lower().split()
    for index, name, _ in devices:
        if all(word in name.lower() for word in device_words):
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


def is_bigpicture_running():
    big_picture_title = get_big_picture_window_title().lower()
    big_picture_words = big_picture_title.split()
    current_window_titles = [title.lower() for title in gw.getAllTitles()]

    for window_title in current_window_titles:
        if all(word in window_title for word in big_picture_words):
            return True

    return False


def write_current_mode(current_mode):
    file_path = get_mode_file_path()
    if current_mode == Mode.GAMEMODE:
        open(file_path, "w").close()
    elif current_mode == Mode.DESKTOP and os.path.exists(file_path):
        os.remove(file_path)


def manage_startup_shortcut(state):
    target_path = os.path.join(os.getcwd(), "BigPictureTV.exe")
    startup_folder = winshell.startup()
    shortcut_path = os.path.join(startup_folder, "BigPictureTV.lnk")
    if state:
        winshell.CreateShortcut(
            Path=shortcut_path,
            Target=target_path,
            Icon=(target_path, 0),
            Description="Launch BigPictureTV",
            StartIn=os.path.dirname(target_path),
        )
    elif os.path.exists(shortcut_path):
        os.remove(shortcut_path)


def check_startup_shortcut():
    return os.path.exists(os.path.join(winshell.startup(), "BigPictureTV.lnk"))


def handle_startup_checkbox_state_changed(state):
    manage_startup_shortcut(state)


class BigPictureTV(QMainWindow):
    def __init__(self):
        super().__init__()
        self.ui = Ui_MainWindow()
        self.ui.setupUi(self)
        self.setWindowTitle("BigPictureTV - Settings")
        self.icon = "light" if darkdetect.isDark() else "dark"
        self.setWindowIcon(QIcon(os.path.join(ICONS_FOLDER, f"icon_desktop_{self.icon}.png")))
        self.tray_icon = None
        self.current_mode = None
        self.monitors = None
        self.settings = {}
        self.first_run = False
        self.paused = False
        self.timer = QTimer()
        self.gamemode_screen = "/external"
        self.desktop_screen = "/internal"
        self.load_settings()
        self.initialize_ui()
        self.is_audio_device_cmdlets_installed()
        self.current_mode = read_current_mode()
        self.switch_mode(self.current_mode or Mode.DESKTOP)
        self.tray_icon = self.create_tray_icon()
        self.timer.timeout.connect(self.update_mode)
        self.update_mode_timer_interval(self.settings.get("CheckRate", 1000))
        if self.first_run:
            self.show()
            self.first_run = False

    def initialize_ui(self):
        self.save_current_monitor_config()
        self.monitors = self.parse_monitor_config(os.path.join(MONITOR_CONFIG_FILE))
        self.ui.gamemode_video_combobox.clear()
        for section in self.monitors:
            self.ui.gamemode_video_combobox.addItem(self.monitors[section][1])
            self.ui.desktop_video_combobox.addItem(self.monitors[section][1])

        self.ui.disableAudioCheckbox.stateChanged.connect(self.handle_disableaudio_checkbox_state_changed)
        self.ui.startupCheckBox.stateChanged.connect(handle_startup_checkbox_state_changed)
        self.ui.gamemodeEntry.textChanged.connect(self.save_settings)
        self.ui.desktopEntry.textChanged.connect(self.save_settings)
        self.ui.checkRateSpinBox.valueChanged.connect(self.save_settings)
        self.ui.gamemode_video_combobox.currentIndexChanged.connect(self.save_settings)
        self.ui.desktop_video_combobox.currentIndexChanged.connect(self.save_settings)
        self.ui.startupCheckBox.setChecked(check_startup_shortcut())

        self.apply_settings()

    def handle_disableaudio_checkbox_state_changed(self, state):
        self.toggle_audio_settings(not state)
        self.save_settings()

    def toggle_audio_settings(self, enabled):
        self.ui.desktopEntry.setEnabled(enabled)
        self.ui.desktopLabel.setEnabled(enabled)
        self.ui.gamemodeEntry.setEnabled(enabled)
        self.ui.gamemodeLabel.setEnabled(enabled)
        self.ui.audioOutputLabel.setEnabled(enabled)

    def create_default_settings(self):
        self.settings = {
            "GAMEMODE_AUDIO": "TV",
            "DESKTOP_AUDIO": "Headset",
            "DisableAudioSwitch": False,
            "CheckRate": 1000,
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
            with open(SETTINGS_FILE, "r") as f:
                self.settings = json.load(f)
        self.apply_settings()

    def apply_settings(self):
        self.ui.gamemodeEntry.setText(self.settings.get("GAMEMODE_AUDIO", ""))
        self.ui.desktopEntry.setText(self.settings.get("DESKTOP_AUDIO", ""))
        self.ui.disableAudioCheckbox.setChecked(self.settings.get("DisableAudioSwitch", False))
        self.ui.checkRateSpinBox.setValue(self.settings.get("CheckRate", 1000))
        self.toggle_audio_settings(not self.ui.disableAudioCheckbox.isChecked())

        saved_gamemode_video = self.settings.get("GAMEMODE_VIDEO", "")
        saved_desktop_video = self.settings.get("DESKTOP_VIDEO", "")
        gamemode_index = self.ui.gamemode_video_combobox.findText(saved_gamemode_video)
        desktop_index = self.ui.gamemode_video_combobox.findText(saved_desktop_video)
        if gamemode_index != -1:
            self.ui.gamemode_video_combobox.setCurrentIndex(gamemode_index)
        if desktop_index != -1:
            self.ui.gamemode_video_combobox.setCurrentIndex(desktop_index)

    def save_settings(self):
        self.settings = {
            "GAMEMODE_AUDIO": self.ui.gamemodeEntry.text(),
            "DESKTOP_AUDIO": self.ui.desktopEntry.text(),
            "DisableAudioSwitch": self.ui.disableAudioCheckbox.isChecked(),
            "CheckRate": self.ui.checkRateSpinBox.value(),
            "GAMEMODE_VIDEO": self.ui.gamemode_video_combobox.currentText(),
            "DESKTOP_VIDEO": self.ui.gamemode_video_combobox.currentText(),
        }
        os.makedirs(os.path.dirname(SETTINGS_FILE), exist_ok=True)
        with open(SETTINGS_FILE, "w") as f:
            json.dump(self.settings, f, indent=4)

    def switch_mode(self, mode):
        if mode == self.current_mode:
            return
        self.current_mode = mode
        self.switch_screen(self.gamemode_screen if mode == Mode.GAMEMODE else self.desktop_screen)
        if not self.ui.disableAudioCheckbox.isChecked():
            switch_audio(
                self.settings.get("GAMEMODE_AUDIO") if mode == Mode.GAMEMODE else self.settings.get("DESKTOP_AUDIO")
            )
        write_current_mode(mode)
        if self.tray_icon:
            self.update_tray_icon_menu()
            self.update_tray_icon()

    def switch_screen(self, mode):
        self.save_current_monitor_config()
        self.monitors = self.parse_monitor_config(os.path.join(MONITOR_CONFIG_FILE))

        if mode == self.gamemode_screen:
            for section in self.monitors:
                if self.monitors[section][1] == self.ui.gamemode_video_combobox.currentText():
                    subprocess.run([MULTIMONITORTOOL_PATH, "/Enable", self.monitors[section][0]])
                    subprocess.run([MULTIMONITORTOOL_PATH, "/SetPrimary", self.monitors[section][0]])
                    subprocess.run([MULTIMONITORTOOL_PATH, "/MoveWindow", "primary", "all"])
        else:
            for section in self.monitors:
                if self.monitors[section][1] == self.ui.desktop_video_combobox.currentText():
                    subprocess.run([MULTIMONITORTOOL_PATH, "/Enable", self.monitors[section][0]])
                    subprocess.run([MULTIMONITORTOOL_PATH, "/SetPrimary", self.monitors[section][0]])
                    subprocess.run([MULTIMONITORTOOL_PATH, "/MoveWindow", "primary", "all"])

    def save_current_monitor_config(self):
        print("Saving current monitor config")
        subprocess.run([MULTIMONITORTOOL_PATH, "/SaveConfig", MONITOR_CONFIG_FILE, "desktop_monitor_config.cfg"])

    def load_current_monitor_config(self):
        print("Loading current monitor config")
        subprocess.run([MULTIMONITORTOOL_PATH, "/LoadConfig", MONITOR_CONFIG_FILE, "desktop_monitor_config.cfg"])

    def extract_monitor_id(self, monitor_id):
        match = re.search(r"MONITOR\\([^\\]+)", monitor_id)
        if match:
            return match.group(1)
        return None

    def parse_monitor_config(self, file_path):
        print("Parsing monitor config")
        config = configparser.ConfigParser()
        config.read(file_path)

        monitors = {}

        for section in config.sections():
            if section.startswith("Monitor"):
                name = config.get(section, "Name")
                monitor_id = config.get(section, "MonitorID")
                skg_code = self.extract_monitor_id(monitor_id)

                monitors[section] = (name, skg_code)

        return monitors

    def is_audio_device_cmdlets_installed(self):
        cmd = 'powershell "Get-Module -ListAvailable -Name AudioDeviceCmdlets"'
        result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
        if "AudioDeviceCmdlets" in result.stdout:
            return True
        self.ui.disableAudioCheckbox.setChecked(True)
        self.ui.disableAudioCheckbox.setEnabled(False)
        self.toggle_audio_settings(False)
        return False

    def update_mode(self):
        if is_bigpicture_running() and self.current_mode != Mode.GAMEMODE:
            self.switch_mode(Mode.GAMEMODE)
        elif not is_bigpicture_running() and self.current_mode != Mode.DESKTOP:
            self.switch_mode(Mode.DESKTOP)

    def update_mode_timer_interval(self, check_rate):
        self.timer.setInterval(check_rate)
        self.timer.start()

    def create_tray_icon(self):
        theme = "light" if darkdetect.isDark() else "dark"
        tray_icon = QSystemTrayIcon(QIcon(os.path.join(ICONS_FOLDER, f"icon_desktop_{theme}.png")))
        tray_icon.setToolTip("BigPictureTV")
        tray_icon.setContextMenu(self.create_menu())
        tray_icon.show()
        return tray_icon

    def create_menu(self):
        menu = QMenu()
        mode_action = QAction(f"Current mode: {self.current_mode.name}", menu)
        mode_action.setEnabled(False)
        menu.addAction(mode_action)
        tray_state_action = QAction(f'Detection state: {"Paused" if self.paused else "Active"}', menu)
        tray_state_action.setEnabled(False)
        menu.addAction(tray_state_action)
        menu.addSeparator()
        pause_resume_action = QAction("Resume detection" if self.paused else "Pause detection", menu)
        pause_resume_action.triggered.connect(self.toggle_detection)
        menu.addAction(pause_resume_action)
        settings_action = QAction("Settings", menu)
        settings_action.triggered.connect(self.show)
        menu.addAction(settings_action)
        exit_action = QAction("Exit", menu)
        exit_action.triggered.connect(QApplication.quit)
        menu.addAction(exit_action)
        return menu

    def update_tray_icon_menu(self):
        self.tray_icon.setContextMenu(self.create_menu())

    def update_tray_icon(self):
        theme = "light" if darkdetect.isDark() else "dark"

        if self.current_mode == Mode.GAMEMODE:
            self.tray_icon.setIcon(QIcon(os.path.join(ICONS_FOLDER, f"icon_gamemode_{theme}.png")))
        else:
            self.tray_icon.setIcon(QIcon(os.path.join(ICONS_FOLDER, f"icon_desktop_{theme}.png")))

    def toggle_detection(self):
        self.paused = not self.paused
        if self.paused:
            self.timer.stop()
            self.tray_icon.setToolTip("BigPictureTV (Paused)")
        else:
            self.timer.start()
            self.tray_icon.setToolTip("BigPictureTV")
        self.update_tray_icon_menu()

    def closeEvent(self, event):
        event.ignore()
        self.hide()


if __name__ == "__main__":
    shared_memory = QSharedMemory("BigPictureTVSharedMemory")
    if shared_memory.attach() or not shared_memory.create(1):
        sys.exit(0)
    app = QApplication(sys.argv)
    big_picture_tv = BigPictureTV()
    sys.exit(app.exec())
