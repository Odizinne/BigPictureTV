import pygetwindow as gw
from steam_language_reader import get_big_picture_window_title


def is_bigpicture_running():
    big_picture_title = get_big_picture_window_title().lower()
    big_picture_words = big_picture_title.split()
    current_window_titles = [title.lower() for title in gw.getAllTitles()]

    for window_title in current_window_titles:
        if all(word in window_title for word in big_picture_words):
            return True

    return False
