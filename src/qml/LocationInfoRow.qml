import QtQuick
import QtQuick.Controls.Material

import Munadi

Item {
    property string address

    implicitHeight: textId.visible ? textId.height : 0

    Text {
        anchors.centerIn: parent
        color: Material.secondaryTextColor
        elide: Text.ElideRight
        font.italic: true
        font.pointSize: 10
        font.underline: textId.font.underline
        text: qsTr("Set your location ...")
        visible: !LocationEngine.isLocationSet
    }

    Text {
        id: textId

        anchors.centerIn: parent
        color: Material.primaryTextColor
        elide: Text.ElideRight
        font.pointSize: 10
        horizontalAlignment: Text.AlignHCenter
        text: parent.address
        verticalAlignment: Text.AlignVCenter
        visible: LocationEngine.isLocationSet
        width: parent.width - 10
        wrapMode: Text.Wrap
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        hoverEnabled: true

        onClicked: {
            locationPopup.open();
            settingsPopup.close();
        }
        onHoveredChanged: textId.font.underline = containsMouse
    }
}
