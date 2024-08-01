import os
from enum import Enum


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


def write_current_mode(current_mode):
    file_path = get_mode_file_path()
    if current_mode == Mode.GAMEMODE:
        open(file_path, "w").close()
    elif current_mode == Mode.DESKTOP and os.path.exists(file_path):
        os.remove(file_path)
