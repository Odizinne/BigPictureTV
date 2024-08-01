import os
import winshell


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
