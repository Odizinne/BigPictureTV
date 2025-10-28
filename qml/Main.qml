pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.FluentWinUI3
import QtQuick.Layouts
import Odizinne.BigPictureTV

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

                Button {
                    Layout.preferredWidth: 170
                    text: qsTr("Reset defaults")
                    onClicked: AppConfiguration.resetToDefaults()
                }
            }
        }

        CustomStackView {
            id: stackView
            Layout.fillWidth: true
            Layout.fillHeight: true
            initialItem: generalPane

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
        GeneralPane {}
    }

    Component {
        id: audioPane
        AudioPane {}
    }

    Component {
        id: videoPane
        VideoPane {}
    }

    Component {
        id: actionsPane
        ActionsPane {}
    }

    Tray {
        onShowRequested: {
            root.show()
        }
        onToggleVisibility: {
            if (root.visible) {
                root.close()
            } else {
                root.show()
            }
        }
    }
}
