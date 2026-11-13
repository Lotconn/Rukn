import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

Button {
    id: control

    property alias horizontalAlignment: textId.horizontalAlignment

    hoverEnabled: true

    background: Item {
        implicitWidth: 100
        opacity: enabled ? 1 : 0.3
    }
    contentItem: Text {
        id: textId

        color: Material.secondaryTextColor
        elide: Text.ElideRight
        font: control.font
        opacity: enabled ? 1.0 : 0.3
        text: control.text
        verticalAlignment: Text.AlignVCenter
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        hoverEnabled: true

        onHoveredChanged: textId.font.underline = containsMouse
        onPressed: mouse => mouse.accepted = false
    }
}
