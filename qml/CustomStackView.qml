import QtQuick
import QtQuick.Controls.FluentWinUI3

StackView {
    id: stackView

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
}
