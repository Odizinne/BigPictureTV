import os
import pygetwindow as gw
from steam_language_reader import get_big_picture_window_title

SUNSHINE_STATUS_FILE = os.path.join(os.environ.get("APPDATA"), "sunshine-status", "status.txt")


def is_bigpicture_running():
    big_picture_title = get_big_picture_window_title().lower()
    big_picture_words = big_picture_title.split()
    current_window_titles = [title.lower() for title in gw.getAllTitles()]

    for window_title in current_window_titles:
        if all(word in window_title for word in big_picture_words):
            return True

    return False


def is_sunshine_stream_active():
    # To be used with https://github.com/Odizinne/Sunshine-Toolbox
    return os.path.exists("SUNSHINE_STATUS_FILE")
