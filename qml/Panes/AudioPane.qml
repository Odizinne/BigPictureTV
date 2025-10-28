import QtQuick
import QtQuick.Controls.FluentWinUI3
import QtQuick.Layouts
import Odizinne.BigPictureTV

Pane {
    ScrollView {
        anchors.fill: parent

        ColumnLayout {
            width: parent.width
            spacing: 3

            Card {
                Layout.fillWidth: true
                title: "Disable audio switching"
                additionalControl: Switch {
                    id: disableAudioCheckBox
                    checked: AppConfiguration.disableAudioSwitch
                    onToggled: AppConfiguration.disableAudioSwitch = checked
                }
            }

            Card {
                Layout.fillWidth: true
                title: "Use HDMI audio for gamemode"
                description: "Will try to detect new audio sources that result from screen activation"
                enabled: !AppConfiguration.disableAudioSwitch
                additionalControl: Switch {
                    id: useHdmiAudioCheckBox
                    checked: AppConfiguration.useHdmiAudioForGamemode
                    onToggled: AppConfiguration.useHdmiAudioForGamemode = checked
                }
            }

            Card {
                Layout.fillWidth: true
                title: "Gamemode audio"
                enabled: !AppConfiguration.disableAudioSwitch && !AppConfiguration.useHdmiAudioForGamemode
                additionalControl: CustomComboBox {
                    id: gamemodeAudioComboBox
                    Layout.fillWidth: true
                    textRole: "name"
                    valueRole: "name"
                    model: AppBridge.audioDevices
                    Component.onCompleted: {
                        var idx = find(AppConfiguration.gamemodeAudioDevice)
                        if (idx >= 0) currentIndex = idx
                    }
                    onActivated: {
                        AppConfiguration.gamemodeAudioDevice = currentText
                        AppConfiguration.gamemodeAudioDeviceId = AppBridge.getDeviceIdFromName(currentText)
                    }
                }
            }

            Card {
                Layout.fillWidth: true
                title: "Desktop audio"
                enabled: !AppConfiguration.disableAudioSwitch
                additionalControl: CustomComboBox {
                    id: desktopAudioComboBox
                    Layout.fillWidth: true
                    textRole: "name"
                    valueRole: "name"
                    model: AppBridge.audioDevices
                    Component.onCompleted: {
                        var idx = find(AppConfiguration.desktopAudioDevice)
                        if (idx >= 0) currentIndex = idx
                    }
                    onActivated: {
                        AppConfiguration.desktopAudioDevice = currentText
                        AppConfiguration.desktopAudioDeviceId = AppBridge.getDeviceIdFromName(currentText)
                    }
                }
            }
        }
    }
}
