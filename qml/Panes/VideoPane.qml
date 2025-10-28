pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.FluentWinUI3
import QtQuick.Layouts
import Odizinne.BigPictureTV

Pane {
    Component.onCompleted: {
        if (!AppConfiguration.disableMonitorSwitch &&
            DisplayManager.displays.length > 0 &&
            AppConfiguration.gamemodeDisplayDevice === "") {
            AppConfiguration.gamemodeDisplayDevice = DisplayManager.displays[0].devicePath
        }
    }

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
                text: qsTr("Gamemode Display")
                font.pixelSize: 18
                font.bold: true
            }

            Label {
                Layout.fillWidth: true
                Layout.leftMargin: 3
                Layout.bottomMargin: 5
                text: qsTr("Select which display to use when entering gamemode")
                font.pixelSize: 13
                opacity: 0.7
                wrapMode: Text.WordWrap
            }

            Card {
                Layout.fillWidth: true
                Layout.preferredWidth: parent.width
                title: "Disable monitor switching"
                additionalControl: Switch {
                    id: disableMonitorCheckBox
                    checked: AppConfiguration.disableMonitorSwitch
                    onToggled: {
                        if (checked) {
                            AppConfiguration.disableMonitorSwitch = true
                            AppConfiguration.gamemodeDisplayDevice = ""
                        } else {
                            AppConfiguration.disableMonitorSwitch = false
                            if (DisplayManager.displays.length > 0) {
                                AppConfiguration.gamemodeDisplayDevice = DisplayManager.displays[0].devicePath
                            }
                        }
                    }
                }
            }

            Repeater {
                model: DisplayManager.displays

                Card {
                    id: monitorDel
                    required property var model
                    Layout.fillWidth: true
                    Layout.preferredWidth: parent.width
                    title: model.name + (model.isActive ? qsTr(" (Currently Active)") : "")
                    additionalControl: Switch {
                        checked: AppConfiguration.gamemodeDisplayDevice === monitorDel.model.devicePath
                        onToggled: {
                            if (checked) {
                                AppConfiguration.disableMonitorSwitch = false
                                AppConfiguration.gamemodeDisplayDevice = monitorDel.model.devicePath
                            } else if (AppConfiguration.gamemodeDisplayDevice === monitorDel.model.devicePath) {
                                checked = true
                            }
                        }
                    }
                }
            }

            Label {
                Layout.fillWidth: true
                Layout.topMargin: 20
                Layout.leftMargin: 3
                text: qsTr("Display Resolution")
                font.pixelSize: 18
                font.bold: true
            }

            Label {
                Layout.fillWidth: true
                Layout.leftMargin: 3
                Layout.bottomMargin: 5
                text: qsTr("Configure the resolution and refresh rate to use in gamemode\nIt will fail if the resolution is incorrect")
                font.pixelSize: 13
                opacity: 0.7
                wrapMode: Text.WordWrap
            }

            Card {
                Layout.fillWidth: true
                Layout.preferredWidth: parent.width
                title: "Width"
                additionalControl: SpinBox {
                    id: widthSpinBox
                    from: 640
                    to: 7680
                    stepSize: 1
                    value: AppConfiguration.gamemodeDisplayWidth
                    onValueModified: {
                        AppConfiguration.gamemodeDisplayWidth = value
                    }
                    editable: true
                }
            }

            Card {
                Layout.fillWidth: true
                Layout.preferredWidth: parent.width
                title: "Height"
                additionalControl: SpinBox {
                    id: heightSpinBox
                    from: 480
                    to: 4320
                    stepSize: 1
                    value: AppConfiguration.gamemodeDisplayHeight
                    onValueModified: {
                        AppConfiguration.gamemodeDisplayHeight = value
                    }
                    editable: true
                }
            }

            Card {
                Layout.fillWidth: true
                Layout.preferredWidth: parent.width
                title: "Refresh Rate (Hz)"
                additionalControl: SpinBox {
                    id: refreshRateSpinBox
                    from: 24
                    to: 360
                    stepSize: 1
                    value: AppConfiguration.gamemodeDisplayRefreshRate
                    onValueModified: {
                        AppConfiguration.gamemodeDisplayRefreshRate = value
                    }
                    editable: true
                }
            }
        }
    }
}
