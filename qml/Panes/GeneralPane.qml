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
                title: "Target window"
                description: "The window that will trigger gamemode switch"
                additionalControl: ComboBox {
                    id: targetWindowComboBox
                    model: [qsTr("Big Picture"), qsTr("Playnite"), qsTr("Custom")]
                    currentIndex: AppConfiguration.targetWindowMode
                    onActivated: AppConfiguration.targetWindowMode = currentIndex
                }
            }

            Card {
                Layout.fillWidth: true
                enabled: AppConfiguration.targetWindowMode === 2
                title: "Custom window title"
                additionalControl: TextField {
                    id: customWindowLineEdit
                    placeholderText: qsTr("Enter window title...")
                    text: AppConfiguration.customWindowTitle
                    onTextChanged: AppConfiguration.customWindowTitle = text
                }
            }

            Card {
                Layout.fillWidth: true
                title: "Launch at startup"
                description: "BigPictureTV will silently start with your computer"
                additionalControl: Switch {
                    id: startupCheckBox
                    checked: AppConfiguration.launchAtStartup
                    onClicked: AppConfiguration.launchAtStartup = checked
                }
            }

            Card {
                Layout.fillWidth: true
                title: "Skip Big Picture intro"
                description: "Sends an emulated click on BigPicture window to skip the intro"
                additionalControl: Switch {
                    id: skipIntroCheckBox
                    checked: AppConfiguration.skipIntro
                    onClicked: AppConfiguration.skipIntro = checked
                }
            }

            Card {
                Layout.fillWidth: true
                title: "Do not switch if Sunshine is active"
                description: "Listen for Sunshine default udp ports to monitor if a stream is active"
                additionalControl: Switch {
                    id: sunshineCheckBox
                    checked: AppConfiguration.doNotSwitchIfSunshineActive
                    onClicked: AppConfiguration.doNotSwitchIfSunshineActive = checked
                }
            }
        }
    }
}
