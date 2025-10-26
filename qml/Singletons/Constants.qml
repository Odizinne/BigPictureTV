pragma Singleton
import QtQuick

Item {
    property bool darkMode: palette.window.hslLightness < 0.5
    property color accentColor: darkMode ? palette.accent : palette.highlight
    property color footerColor: darkMode ? "#1c1c1c" : "#eeeeee"
    property color footerBorderColor: darkMode ? "#0F0F0F" : "#A0A0A0"
    property color panelColor: darkMode ? "#242424" : "#f2f2f2"
    property color cardColor: darkMode ? "#2b2b2b" : "#fbfbfb"
    property color cardBorderColor: darkMode ? "#1d1d1d" : "#e5e5e5"
    property color separatorColor: darkMode ? "#E3E3E3" : "#A0A0A0"
}
