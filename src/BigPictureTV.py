import sys
import os
import json
import darkdetect
from PyQt6.QtWidgets import QApplication, QSystemTrayIcon, QMenu, QMainWindow
from PyQt6.QtGui import QIcon, QAction
from PyQt6.QtCore import QTimer, QSharedMemory
from design import Ui_MainWindow
from monitor_manager import enable_clone_mode, enable_external_mode, enable_internal_mode
from audio_manager import switch_audio
from mode_manager import Mode, read_current_mode, write_current_mode
from shortcut_manager import check_startup_shortcut, handle_startup_checkbox_state_changed
from utils import (
    is_bigpicture_running,
    is_sunshine_stream_active,
    is_audio_device_cmdlets_installed,
    close_discord,
    start_discord,
    is_discord_installed,
)

SETTINGS_FILE = os.path.join(os.environ["APPDATA"], "BigPictureTV", "settings.json")
ICONS_FOLDER = "icons" if getattr(sys, "frozen", False) else os.path.join(os.path.dirname(__file__), "icons")


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
        self.settings = {}
        self.first_run = False
        self.paused = False
        self.timer = QTimer()
        self.load_settings()
        self.initialize_ui()
        self.get_audio_capabilities()
        self.current_mode = read_current_mode()
        self.switch_mode(self.current_mode or Mode.DESKTOP)
        self.tray_icon = self.create_tray_icon()
        self.timer.timeout.connect(self.update_mode)
        self.update_mode_timer_interval(self.settings.get("CheckRate", 1000))
        if self.first_run:
            self.show()
            self.first_run = False

    def initialize_ui(self):
        self.ui.disableAudioCheckbox.stateChanged.connect(self.handle_disableaudio_checkbox_state_changed)
        self.ui.startupCheckBox.stateChanged.connect(handle_startup_checkbox_state_changed)
        self.ui.gamemodeEntry.textChanged.connect(self.save_settings)
        self.ui.desktopEntry.textChanged.connect(self.save_settings)
        self.ui.checkRateSpinBox.valueChanged.connect(self.save_settings)
        self.ui.startupCheckBox.setChecked(check_startup_shortcut())
        self.ui.clone_checkbox.stateChanged.connect(self.save_settings)
        self.ui.close_discord_checkbox.stateChanged.connect(self.save_settings)

        self.ui.close_discord_checkbox.setEnabled(is_discord_installed())
        self.ui.close_discord_label.setEnabled(is_discord_installed())

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
        self.ui.clone_checkbox.setChecked(self.settings.get("use_clone", False))
        self.ui.close_discord_checkbox.setChecked(self.settings.get("discord_action", False))

    def save_settings(self):
        self.settings = {
            "GAMEMODE_AUDIO": self.ui.gamemodeEntry.text(),
            "DESKTOP_AUDIO": self.ui.desktopEntry.text(),
            "DisableAudioSwitch": self.ui.disableAudioCheckbox.isChecked(),
            "CheckRate": self.ui.checkRateSpinBox.value(),
            "use_clone": self.ui.clone_checkbox.isChecked(),
            "discord_action": self.ui.close_discord_checkbox.isChecked(),
        }
        os.makedirs(os.path.dirname(SETTINGS_FILE), exist_ok=True)
        with open(SETTINGS_FILE, "w") as f:
            json.dump(self.settings, f, indent=4)

    def switch_mode(self, mode):
        if mode == self.current_mode:
            return

        self.current_mode = mode
        gamemode_audio = self.settings.get("GAMEMODE_AUDIO")
        desktop_audio = self.settings.get("DESKTOP_AUDIO")

        self.switch_screen("gamemode" if mode == Mode.GAMEMODE else "desktop")
        close_discord() if mode == Mode.GAMEMODE else start_discord()

        if not self.ui.disableAudioCheckbox.isChecked():
            switch_audio(gamemode_audio if mode == Mode.GAMEMODE else desktop_audio)
        write_current_mode(mode)

        if self.tray_icon:
            self.update_tray_icon_menu()
            self.update_tray_icon()

    def switch_screen(self, screen):
        if screen == "gamemode" and self.ui.clone_checkbox.isChecked():
            enable_clone_mode()
        elif screen == "gamemode" and not self.ui.clone_checkbox.isChecked():
            enable_external_mode()
        elif screen == "desktop":
            enable_internal_mode()

    def get_audio_capabilities(self):
        if is_audio_device_cmdlets_installed() is False:
            self.ui.disableAudioCheckbox.setChecked(True)
            self.ui.disableAudioCheckbox.setEnabled(False)
            self.toggle_audio_settings(False)
            return

    def update_mode(self):
        if is_bigpicture_running() and self.current_mode != Mode.GAMEMODE and not is_sunshine_stream_active():
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
