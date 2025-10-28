import QtQuick
import Qt.labs.platform as Platform

Platform.SystemTrayIcon {
    id: tray
    visible: true
    icon.source: Constants.darkMode ? "qrc:/icons/icon_light.png" : "qrc:/icons/icon_dark.png"
    tooltip: "BigPictureTV"
    signal showRequested()
    signal toggleVisibility()

    onActivated: function(reason) {
        if (reason === Platform.SystemTrayIcon.Trigger) {
            toggleVisibility()
        }
    }

    menu: Platform.Menu {
        Platform.MenuItem {
            text: qsTr("Open")
            onTriggered: tray.showRequested()
        }

        Platform.MenuItem {
            text: qsTr("Exit")
            onTriggered: Qt.quit()
        }
    }
}
