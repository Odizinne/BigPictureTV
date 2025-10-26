pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.FluentWinUI3
import QtQuick.Layouts
import Odizinne.BigPictureTV
import Qt.labs.platform as Platform

ApplicationWindow {
    id: root
    width: 800
    height: 450
    minimumWidth: 800
    minimumHeight: 450
    visible: false
    title: qsTr("BigPictureTV - Settings")

    Component.onCompleted: {
        if (AppConfiguration.firstRun) {
            root.show()
            AppConfiguration.firstRun = false
        }
    }

    ListModel {
        id: navigationModel
        ListElement { name: "General"; icon: "qrc:/icons/general.svg️" }
        ListElement { name: "Audio"; icon: "qrc:/icons/audio.svg" }
        ListElement { name: "Video"; icon: "qrc:/icons/video.svg️" }
        ListElement { name: "Actions"; icon: "qrc:/icons/actions.svg" }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0
        anchors.leftMargin: 10

        Item {
            Layout.topMargin: 10
            Layout.bottomMargin: 10
            Layout.fillHeight: true
            Layout.preferredWidth: 170
            ColumnLayout {
                anchors.fill: parent
                ListView {
                    id: navigationList
                    Layout.fillHeight: true
                    Layout.preferredWidth: 170
                    spacing: 0
                    model: navigationModel
                    currentIndex: 0
                    interactive: false
                    delegate: ItemDelegate {
                        required property var model
                        required property int index
                        width: ListView.view.width
                        height: 40
                        text: model.name
                        //icon.source: model.icon
                        highlighted: ListView.isCurrentItem
                        onClicked: navigationList.currentIndex = index
                    }
                }

                Item {
                    Layout.fillHeight: true
                }

                Button {
                    Layout.preferredWidth: 170
                    text: qsTr("Reset defaults")
                    onClicked: AppConfiguration.resetToDefaults()
                }
            }
        }

        StackView {
            id: stackView
            Layout.fillWidth: true
            Layout.fillHeight: true
            initialItem: generalPane

            popEnter: Transition {
                ParallelAnimation {
                    NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 150; easing.type: Easing.InQuint }
                    NumberAnimation { property: "y"; from: (stackView.mirrored ? -0.3 : 0.3) * -stackView.width; to: 0; duration: 300; easing.type: Easing.OutCubic }
                }
            }

            pushEnter: Transition {
                ParallelAnimation {
                    NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 150; easing.type: Easing.InQuint }
                    NumberAnimation { property: "y"; from: (stackView.mirrored ? -0.3 : 0.3) * stackView.width; to: 0; duration: 300; easing.type: Easing.OutCubic }
                }
            }

            popExit: Transition {
                NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 150; easing.type: Easing.OutQuint }
            }

            pushExit: Transition {
                NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 150; easing.type: Easing.OutQuint }
            }

            replaceEnter: Transition {
                ParallelAnimation {
                    NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 150; easing.type: Easing.InQuint }
                    NumberAnimation { property: "y"; from: (stackView.mirrored ? -0.3 : 0.3) * stackView.width; to: 0; duration: 300; easing.type: Easing.OutCubic }
                }
            }

            Connections {
                target: navigationList
                function onCurrentIndexChanged() {
                    switch (navigationList.currentIndex) {
                    case 0: stackView.push(generalPane); break;
                    case 1: stackView.push(audioPane); break;
                    case 2: stackView.push(videoPane); break;
                    case 3: stackView.push(actionsPane); break;
                    }
                }
            }
        }
    }

    Component {
        id: generalPane

        Pane {
            ScrollView {
                anchors.fill: parent

                ColumnLayout {
                    width: parent.width
                    spacing: 3

                    Card {
                        Layout.fillWidth: true
                        title: "Target window"
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
                        additionalControl: Switch {
                            id: startupCheckBox
                            checked: AppConfiguration.launchAtStartup
                            onClicked: AppConfiguration.launchAtStartup = checked
                        }
                    }

                    Card {
                        Layout.fillWidth: true
                        title: "Skip Big Picture intro"
                        additionalControl: Switch {
                            id: skipIntroCheckBox
                            checked: AppConfiguration.skipIntro
                            onClicked: AppConfiguration.skipIntro = checked
                        }
                    }
                }
            }
        }
    }

    // Audio Pane Component
    Component {
        id: audioPane

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
                        title: "Gamemode audio"
                        enabled: !AppConfiguration.disableAudioSwitch
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
    }

    Component {
        id: videoPane

        Pane {
            ScrollView {
                anchors.fill: parent

                ColumnLayout {
                    width: parent.width
                    spacing: 3

                    Card {
                        Layout.fillWidth: true
                        title: "Disable monitor switching"
                        additionalControl: Switch {
                            id: disableMonitorCheckBox
                            checked: AppConfiguration.disableMonitorSwitch
                            onToggled: AppConfiguration.disableMonitorSwitch = checked
                        }
                    }

                    Card {
                        Layout.fillWidth: true
                        title: "Gamemode monitor"
                        enabled: !AppConfiguration.disableMonitorSwitch
                        additionalControl: CustomComboBox {
                            id: gamemodeMonitorComboBox
                            Layout.fillWidth: true
                            model: [qsTr("External"), qsTr("Clone")]
                            currentIndex: AppConfiguration.gamemodeMonitorMode
                            onActivated: AppConfiguration.gamemodeMonitorMode = currentIndex
                        }
                    }

                    Card {
                        Layout.fillWidth: true
                        title: "Desktop monitor"
                        enabled: !AppConfiguration.disableMonitorSwitch
                        additionalControl: CustomComboBox {
                            id: desktopMonitorComboBox
                            Layout.fillWidth: true
                            model: [qsTr("Internal"), qsTr("Extend")]
                            currentIndex: AppConfiguration.desktopMonitorMode
                            onActivated: AppConfiguration.desktopMonitorMode = currentIndex
                        }
                    }
                }
            }
        }
    }

    Component {
        id: actionsPane

        Pane {
            ScrollView {
                anchors.fill: parent

                ColumnLayout {
                    width: parent.width
                    spacing: 3

                    Card {
                        Layout.fillWidth: true
                        title: "Close Discord when entering gamemode"
                        additionalControl: Switch {
                            id: closeDiscordCheckBox
                            enabled: AppBridge.discordInstalled
                            checked: AppConfiguration.closeDiscordAction
                            onToggled: AppConfiguration.closeDiscordAction = checked
                            ToolTip.visible: !enabled && hovered
                            ToolTip.text: qsTr("Discord does not appear to be installed")
                        }
                    }

                    Card {
                        Layout.fillWidth: true
                        title: "Enable performance power plan"
                        additionalControl: Switch {
                            id: enablePerformancePowerPlan
                            checked: AppConfiguration.performancePowerplanAction
                            onToggled: AppConfiguration.performancePowerplanAction = checked
                            ToolTip.visible: hovered
                            ToolTip.text: qsTr("Will revert to previously set power plan")
                        }
                    }

                    Card {
                        Layout.fillWidth: true
                        title: "Disable Night Light when entering gamemode"
                        additionalControl: Switch {
                            id: disableNightLightCheckBox
                            checked: AppConfiguration.disableNightlightAction
                            onToggled: AppConfiguration.disableNightlightAction = checked
                        }
                    }

                    Card {
                        Layout.fillWidth: true
                        title: "Pause media when entering gamemode"
                        additionalControl: Switch {
                            id: pauseMediaAction
                            checked: AppConfiguration.pauseMediaAction
                            onToggled: AppConfiguration.pauseMediaAction = checked
                        }
                    }

                    Card {
                        Layout.fillWidth: true
                        title: "Enable HDR when entering gamemode"
                        additionalControl: Switch {
                            id: hdrCheckBox
                            enabled: AppBridge.hdrCapability !== 2
                            checked: AppConfiguration.enableHdr
                            onToggled: AppConfiguration.enableHdr = checked
                            ToolTip.visible: !enabled && hovered
                            ToolTip.text: qsTr("HDR is not supported on this system")
                        }
                    }
                }
            }
        }
    }

    Platform.SystemTrayIcon {
        visible: true
        icon.source: Constants.darkMode ? "qrc:/icons/icon_light.png" : "qrc:/icons/icon_dark.png"
        tooltip: "BigPictureTV"
        onActivated: function(reason) {
            if (reason === Platform.SystemTrayIcon.Trigger) {
                if (root.visible) {
                    root.close()
                } else {
                    root.show()
                }
            }
        }

        menu: Platform.Menu {
            Platform.MenuItem {
                text: qsTr("Open")
                onTriggered: root.show()
            }

            Platform.MenuItem {
                text: qsTr("Exit")
                onTriggered: Qt.quit()
            }
        }
    }
}
