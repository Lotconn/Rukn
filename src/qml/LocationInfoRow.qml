import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

Item {

    property string cityName
    property string countryName

    Text {
        anchors.centerIn: parent
        font.pixelSize: parent.height / 2
        font.italic: true
        color: Material.secondaryTextColor
        elide: Text.ElideRight
        text: qsTr("Set your location ...")
        visible: mm.locationNotSet
        font.underline: textId.font.underline
    }
    Text {
        id: textId
        anchors.centerIn: parent
        font.pixelSize: parent.height / 2
        color: Material.primaryTextColor
        elide: Text.ElideRight

        visible: !mm.locationNotSet

        text: `${cityName}, <i>${countryName}</i>`
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true

        cursorShape: Qt.PointingHandCursor

        onClicked: popup.open()
        onHoveredChanged: textId.font.underline = containsMouse
    }
}
