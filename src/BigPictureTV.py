import sys
import os
import json
from PyQt6.QtWidgets import QApplication, QSystemTrayIcon, QMenu, QMainWindow, QMessageBox
from PyQt6.QtGui import QIcon, QAction
from PyQt6.QtCore import Qt, QTimer, QSharedMemory, QTranslator, QLocale
from design import Ui_MainWindow
from tooltiped_slider import TooltipedSlider
from audio_manager import switch_audio
from mode_manager import Mode, read_current_mode, write_current_mode
from shortcut_manager import check_startup_shortcut, handle_startup_checkbox_state_changed
from color_utils import set_frame_color_based_on_window
from utils import (
    is_bigpicture_running,
    is_sunshine_stream_active,
    is_audio_device_cmdlets_installed,
    install_audio_module,
    close_discord,
    start_discord,
    is_discord_installed,
    get_theme,
    run_displayswitch,
    is_windows_10,
)

SETTINGS_FILE = os.path.join(os.environ["APPDATA"], "BigPictureTV", "settings.json")
ICONS_FOLDER = "icons" if getattr(sys, "frozen", False) else os.path.join(os.path.dirname(__file__), "icons")


class BigPictureTV(QMainWindow):
    def __init__(self):
        super().__init__()
        self.ui = Ui_MainWindow()
        self.ui.setupUi(self)
        self.setWindowIcon(QIcon(os.path.join(ICONS_FOLDER, f"icon_desktop_{get_theme()}.png")))
        self.current_mode = read_current_mode()
        self.settings = {}
        self.first_run = False
        self.paused = False
        self.timer = QTimer()
        self.set_fusion_frames()
        self.init_checkrate_tooltipedslider()
        self.populate_comboboxes()
        self.load_settings()
        self.setup_ui_connections()
        self.get_audio_capabilities()
        self.switch_mode(self.current_mode or Mode.DESKTOP)
        self.tray_icon = self.create_tray_icon()
        self.timer.timeout.connect(self.update_mode)
        self.update_mode_timer_interval(self.settings.get("CheckRate", 1000))
        if self.first_run:
            self.show()
            self.first_run = False

    def set_fusion_frames(self):
        if app.style().objectName() == "fusion":
            set_frame_color_based_on_window(self, self.ui.gridFrame)
            set_frame_color_based_on_window(self, self.ui.gridFrame1)
            set_frame_color_based_on_window(self, self.ui.audioFrame)
            set_frame_color_based_on_window(self, self.ui.settingsFrame)

    def setup_ui_connections(self):
        self.ui.disableAudioCheckbox.stateChanged.connect(self.handle_disableaudio_checkbox_state_changed)
        self.ui.startupCheckBox.stateChanged.connect(handle_startup_checkbox_state_changed)
        self.ui.gamemodeEntry.textChanged.connect(self.save_settings)
        self.ui.desktopEntry.textChanged.connect(self.save_settings)
        self.ui.startupCheckBox.setChecked(check_startup_shortcut())
        self.ui.close_discord_checkbox.stateChanged.connect(self.save_settings)
        self.ui.close_discord_checkbox.setEnabled(is_discord_installed())
        self.ui.close_discord_label.setEnabled(is_discord_installed())
        self.ui.gamemode_monitor_combobox.currentIndexChanged.connect(self.save_settings)
        self.ui.desktop_monitor_combobox.currentIndexChanged.connect(self.save_settings)
        self.ui.checkrate_slider.valueChanged.connect(self.handle_checkrate_slider_value_changed)
        self.ui.checkrate_slider.sliderReleased.connect(self.save_settings)
        self.ui.install_audio_button.clicked.connect(self.handle_audio_button_clicked)
        self.ui.disable_monitor_checkbox.stateChanged.connect(self.handle_disablemonitor_checkbox_state_changed)

    def populate_comboboxes(self):
        self.ui.gamemode_monitor_combobox.addItem(self.tr("External"))
        self.ui.gamemode_monitor_combobox.addItem(self.tr("Clone"))
        self.ui.desktop_monitor_combobox.addItem(self.tr("Internal"))
        self.ui.desktop_monitor_combobox.addItem(self.tr("Extend"))

    def init_checkrate_tooltipedslider(self):
        self.checkrate_tooltipedslider = TooltipedSlider(Qt.Orientation.Horizontal, self.ui.centralwidget)
        layout = self.ui.settings_layout
        index = layout.indexOf(self.ui.checkrate_label)
        position = layout.getItemPosition(index)
        row = position[0]
        column = position[1]
        layout.addWidget(self.checkrate_tooltipedslider, row, column + 1)
        self.ui.checkrate_slider.deleteLater()
        self.ui.checkrate_slider = self.checkrate_tooltipedslider
        self.ui.checkrate_slider.setTickInterval(100)
        self.ui.checkrate_slider.setRange(100, 1000)

    def handle_audio_button_clicked(self):
        self.ui.install_audio_button.setEnabled(False)
        status, message = install_audio_module()
        if status == "Success":
            QMessageBox.information(self, status, message)
        else:
            QMessageBox.critical(self, status, message)
            self.ui.install_audio_button.setEnabled(True)

        self.get_audio_capabilities()

    def handle_checkrate_slider_value_changed(self, value):
        remainder = value % 100
        if remainder != 0:
            if remainder < 50:
                value -= remainder
            else:
                value += 100 - remainder
            self.ui.checkrate_slider.setValue(value)

        self.update_mode_timer_interval(value)

    def handle_disableaudio_checkbox_state_changed(self, state):
        self.toggle_audio_settings(not state)
        self.save_settings()

    def handle_disablemonitor_checkbox_state_changed(self, state):
        self.toggle_monitor_settings(not state)
        self.save_settings()

    def toggle_audio_settings(self, enabled):
        self.ui.desktopEntry.setEnabled(enabled)
        self.ui.desktopLabel.setEnabled(enabled)
        self.ui.gamemodeEntry.setEnabled(enabled)
        self.ui.gamemodeLabel.setEnabled(enabled)
        self.ui.audioOutputLabel.setEnabled(enabled)

    def toggle_monitor_settings(self, enabled):
        self.ui.gamemode_monitor_combobox.setEnabled(enabled)
        self.ui.desktop_monitor_combobox.setEnabled(enabled)
        self.ui.label.setEnabled(enabled)
        self.ui.label_7.setEnabled(enabled)
        self.ui.label_8.setEnabled(enabled)

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
        self.ui.checkrate_slider.setValue(self.settings.get("CheckRate", 1000))
        self.toggle_audio_settings(not self.ui.disableAudioCheckbox.isChecked())
        self.ui.close_discord_checkbox.setChecked(self.settings.get("discord_action", False))
        self.ui.gamemode_monitor_combobox.setCurrentIndex(self.settings.get("GAMEMODE_MONITOR", 0))
        self.ui.desktop_monitor_combobox.setCurrentIndex(self.settings.get("DESKTOP_MONITOR", 0))
        self.ui.disable_monitor_checkbox.setChecked(self.settings.get("DisableMonitorSwitch", False))
        self.toggle_monitor_settings(not self.ui.disable_monitor_checkbox.isChecked())

    def save_settings(self):
        self.settings = {
            "GAMEMODE_AUDIO": self.ui.gamemodeEntry.text(),
            "DESKTOP_AUDIO": self.ui.desktopEntry.text(),
            "DisableAudioSwitch": self.ui.disableAudioCheckbox.isChecked(),
            "CheckRate": self.ui.checkrate_slider.value(),
            "discord_action": self.ui.close_discord_checkbox.isChecked(),
            "GAMEMODE_MONITOR": self.ui.gamemode_monitor_combobox.currentIndex(),
            "DESKTOP_MONITOR": self.ui.desktop_monitor_combobox.currentIndex(),
            "DisableMonitorSwitch": self.ui.disable_monitor_checkbox.isChecked(),
        }
        os.makedirs(os.path.dirname(SETTINGS_FILE), exist_ok=True)
        with open(SETTINGS_FILE, "w") as f:
            json.dump(self.settings, f, indent=4)

    def switch_mode(self, mode):
        if mode == self.current_mode:
            return

        self.current_mode = mode
        if not self.ui.disable_monitor_checkbox.isChecked():
            if self.current_mode == Mode.GAMEMODE and self.ui.gamemode_monitor_combobox.currentIndex() == 0:
                monitor_mode = "/external"
            elif self.current_mode == Mode.GAMEMODE and self.ui.gamemode_monitor_combobox.currentIndex() == 1:
                monitor_mode = "/clone"
            elif self.current_mode == Mode.DESKTOP and self.ui.desktop_monitor_combobox.currentIndex() == 0:
                monitor_mode = "/internal"
            elif self.current_mode == Mode.DESKTOP and self.ui.desktop_monitor_combobox.currentIndex() == 1:
                monitor_mode = "/extend"
            run_displayswitch(monitor_mode)

        if self.ui.close_discord_checkbox.isChecked():
            close_discord() if self.current_mode == Mode.GAMEMODE else start_discord()

        if not self.ui.disableAudioCheckbox.isChecked():
            gamemode_audio = self.settings.get("GAMEMODE_AUDIO")
            desktop_audio = self.settings.get("DESKTOP_AUDIO")
            switch_audio(gamemode_audio if self.current_mode == Mode.GAMEMODE else desktop_audio)

        write_current_mode(self.current_mode)

        if self.tray_icon:
            variant = "gamemode" if self.current_mode == Mode.GAMEMODE else "desktop"
            self.tray_icon.setIcon(QIcon(os.path.join(ICONS_FOLDER, f"icon_{variant}_{get_theme()}.png")))
            self.tray_icon.setContextMenu(self.create_menu())

    def get_audio_capabilities(self):
        if not is_audio_device_cmdlets_installed():
            self.ui.disableAudioCheckbox.setChecked(True)
            self.ui.disableAudioCheckbox.setEnabled(False)
            self.toggle_audio_settings(False)
        else:
            self.ui.disableAudioCheckbox.setEnabled(True)
            self.ui.install_audio_button.setVisible(False)
            if not self.ui.disableAudioCheckbox.isChecked():
                self.toggle_audio_settings(True)
            self.adjustSize()

    def update_mode(self):
        if is_bigpicture_running() and self.current_mode != Mode.GAMEMODE and not is_sunshine_stream_active():
            self.switch_mode(Mode.GAMEMODE)
        elif not is_bigpicture_running() and self.current_mode != Mode.DESKTOP:
            self.switch_mode(Mode.DESKTOP)

    def update_mode_timer_interval(self, check_rate):
        self.timer.setInterval(check_rate)
        self.timer.start()

    def create_tray_icon(self):
        tray_icon = QSystemTrayIcon(QIcon(os.path.join(ICONS_FOLDER, f"icon_desktop_{get_theme()}.png")))
        tray_icon.setToolTip("BigPictureTV")
        tray_icon.setContextMenu(self.create_menu())
        tray_icon.show()
        return tray_icon

    def create_menu(self):
        menu = QMenu()
        pause_resume_action = QAction(self.tr("Resume detection") if self.paused else self.tr("Pause detection"), menu)
        pause_resume_action.triggered.connect(self.toggle_detection)
        settings_action = QAction(self.tr("Settings"), menu)
        settings_action.triggered.connect(self.show)
        exit_action = QAction(self.tr("Exit"), menu)
        exit_action.triggered.connect(QApplication.quit)

        menu.addAction(pause_resume_action)
        menu.addAction(settings_action)
        menu.addAction(exit_action)

        return menu

    def toggle_detection(self):
        self.paused = not self.paused
        if self.paused:
            self.timer.stop()
            self.tray_icon.setToolTip("BigPictureTV (Paused)")
        else:
            self.timer.start()
            self.tray_icon.setToolTip("BigPictureTV")
        self.tray_icon.setContextMenu(self.create_menu())

    def closeEvent(self, event):
        event.ignore()
        self.hide()


if __name__ == "__main__":
    shared_memory = QSharedMemory("BigPictureTVSharedMemory")
    if shared_memory.attach() or not shared_memory.create(1):
        sys.exit(0)

    app = QApplication(sys.argv)
    if is_windows_10():
        app.setStyle("Fusion")

    translator = QTranslator()
    locale_name = QLocale.system().name()
    locale = locale_name[:2]
    if locale:
        file_name = f"tr/bigpicturetv_{locale}.qm"
    else:
        file_name = None

    if file_name and translator.load(file_name):
        app.installTranslator(translator)

    big_picture_tv = BigPictureTV()
    sys.exit(app.exec())
