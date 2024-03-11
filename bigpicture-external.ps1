# Install the AudioDeviceCmdlets module if not already installed
if (-not (Get-Module -Name AudioDeviceCmdlets -ListAvailable)) {
    Install-Module -Name AudioDeviceCmdlets -Scope CurrentUser -Force
}

# Function to enable/disable external screen and switch audio output
function Toggle-ExternalScreen {
    param (
        [bool]$Enable
    )

    if ($Enable) {
        # Command to enable external screen
        DisplaySwitch.exe /external
        Write-Host "Enabling external screen..."

        # Switch audio output to HDMI
        Set-AudioDevice -Playback -DeviceName "1 - SAMSUNG (AMD High Definition Audio Device)"
        Write-Host "Switched audio output to HDMI."
    } else {
        # Command to disable external screen
        DisplaySwitch.exe /internal
        Write-Host "Disabling external screen..."

        # Switch audio output to headset
        Set-AudioDevice -Playback -DeviceName "Headset Earphone (3- CORSAIR VOID ELITE Wireless Gaming Dongle)"
        Write-Host "Switched audio output to headset."
    }
}

# Main loop
while ($true) {
    $bigPictureWindowExists = Get-Process | Where-Object { $_.MainWindowTitle -like "*steam*mode*big*picture*" }

    if ($bigPictureWindowExists) {
        if (-not $externalScreenEnabled) {
            Toggle-ExternalScreen -Enable $true
            $externalScreenEnabled = $true
            Write-Host "External screen enabled."
        }
    } else {
        if ($externalScreenEnabled) {
            Toggle-ExternalScreen -Enable $false
            $externalScreenEnabled = $false
            Write-Host "External screen disabled."
        }
    }

    # Sleep for some time before checking again
    Start-Sleep -Seconds 1
}
