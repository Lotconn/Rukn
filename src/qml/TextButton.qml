import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

Button {
    id: control

    hoverEnabled: true
    property alias horizontalAlignment: textId.horizontalAlignment

    contentItem: Text {
        id: textId
        text: control.text
        font: control.font
        opacity: enabled ? 1.0 : 0.3
        color: Material.secondaryTextColor
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Item {
        implicitWidth: 100
        opacity: enabled ? 1 : 0.3
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onHoveredChanged: textId.font.underline = containsMouse
        onPressed: mouse.accepted = false
    }
}
