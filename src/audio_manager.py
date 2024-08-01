import subprocess
import re
import time


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


def is_audio_device_cmdlets_installed():
    cmd = 'powershell "Get-Module -ListAvailable -Name AudioDeviceCmdlets"'
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
    if "AudioDeviceCmdlets" in result.stdout:
        return True
    else:
        return False
