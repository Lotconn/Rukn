import QtQuick 2.5
import QtQuick.Controls 2.0

Button {
    id: control

    hoverEnabled: true
    anchors.margins:parent.height / 10
    font.pixelSize: parent.height / 3

    contentItem: Text {
        text: control.text
        font: control.font
        opacity: enabled ? 1.0 : 0.3
        color: theme.fontColour
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight

    }

    background: Rectangle {
        implicitWidth: 100
        implicitHeight: 40
        opacity: enabled ? 1 : 0.3
        color: control.pressed ? theme.borderColour : "white"
        border.color: theme.borderColour
        border.width: control.hovered ? 1 : 0
    }
}
