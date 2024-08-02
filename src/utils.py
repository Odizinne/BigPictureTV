import os
import subprocess
import psutil
import pygetwindow as gw
from steam_language_reader import get_big_picture_window_title

SUNSHINE_STATUS_FILE = os.path.join(os.environ.get("APPDATA"), "sunshine-status", "status.txt")
DISCORD_EXE = os.path.join(os.environ.get("LOCALAPPDATA"), "Discord", "Update.exe")


def is_audio_device_cmdlets_installed():
    cmd = 'powershell "Get-Module -ListAvailable -Name AudioDeviceCmdlets"'
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
    if "AudioDeviceCmdlets" in result.stdout:
        return True
    else:
        return False


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
    return os.path.exists(SUNSHINE_STATUS_FILE)


def close_discord():
    for proc in psutil.process_iter():
        try:
            if "discord" in proc.name().lower():
                proc.kill()
        except (psutil.NoSuchProcess, psutil.AccessDenied, psutil.ZombieProcess):
            pass


def start_discord():
    try:
        subprocess.Popen([DISCORD_EXE, "--processStart", "Discord.exe", "--process-start-args", "--start-minimized"])
    except FileNotFoundError:
        pass


def is_discord_installed():
    return os.path.exists(DISCORD_EXE)