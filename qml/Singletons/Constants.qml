pragma Singleton
import QtQuick

Item {
    readonly property bool darkMode: Qt.application.styleHints.colorScheme === Qt.ColorScheme.Dark
    property color cardColor: darkMode ? "#2b2b2b" : "#fbfbfb"
    property color cardBorderColor: darkMode ? "#1d1d1d" : "#e5e5e5"
}
