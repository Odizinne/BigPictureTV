import QtQuick
import QtQuick.Controls.FluentWinUI3
import QtQuick.Layouts
import Odizinne.BigPictureTV

Pane {
    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth

        ColumnLayout {
            width: parent.width
            spacing: 3

            Label {
                Layout.fillWidth: true
                Layout.topMargin: 10
                Layout.leftMargin: 3
                text: qsTr("Actions when entering Gamemode")
                font.pixelSize: 18
                font.bold: true
            }

            Label {
                Layout.fillWidth: true
                Layout.leftMargin: 3
                Layout.bottomMargin: 5
                text: qsTr("Will be reverted on entering desktop mode")
                font.pixelSize: 13
                opacity: 0.7
                wrapMode: Text.WordWrap
            }

            Card {
                Layout.fillWidth: true
                Layout.preferredWidth: parent.width
                enabled: AppBridge.discordInstalled
                title: "Close Discord"
                additionalControl: Switch {
                    id: closeDiscordCheckBox
                    checked: AppConfiguration.closeDiscordAction
                    onToggled: AppConfiguration.closeDiscordAction = checked
                    ToolTip.visible: !enabled && hovered
                    ToolTip.text: qsTr("Discord does not appear to be installed")
                }
            }

            Card {
                Layout.fillWidth: true
                Layout.preferredWidth: parent.width
                title: "Enable performance power plan"
                additionalControl: Switch {
                    id: enablePerformancePowerPlan
                    checked: AppConfiguration.performancePowerplanAction
                    onToggled: AppConfiguration.performancePowerplanAction = checked
                }
            }

            Card {
                Layout.fillWidth: true
                Layout.preferredWidth: parent.width
                title: "Disable Night Light"
                additionalControl: Switch {
                    id: disableNightLightCheckBox
                    checked: AppConfiguration.disableNightlightAction
                    onToggled: AppConfiguration.disableNightlightAction = checked
                }
            }

            Card {
                Layout.fillWidth: true
                Layout.preferredWidth: parent.width
                title: "Pause media"
                additionalControl: Switch {
                    id: pauseMediaAction
                    checked: AppConfiguration.pauseMediaAction
                    onToggled: AppConfiguration.pauseMediaAction = checked
                }
            }

            Card {
                Layout.fillWidth: true
                Layout.preferredWidth: parent.width
                title: "Enable HDR"
                enabled: AppBridge.hdrCapability !== 2
                additionalControl: Switch {
                    id: hdrCheckBox
                    checked: AppConfiguration.enableHdr
                    onToggled: AppConfiguration.enableHdr = checked
                    ToolTip.visible: !enabled && hovered
                    ToolTip.text: qsTr("HDR is not supported on this system")
                }
            }
        }
    }
}
