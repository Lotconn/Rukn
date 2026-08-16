import QtQuick
// import QtQuick.Controls
import QtQuick.Controls.Material

// import QtQuick.Layouts

Item {
    property string cityName
    property string countryName

    Text {
        anchors.centerIn: parent
        color: Material.secondaryTextColor
        elide: Text.ElideRight
        font.italic: true
        font.pixelSize: parent.height / 2
        font.underline: textId.font.underline
        text: qsTr("Set your location ...")
        visible: mm.locationNotSet
    }

    Text {
        id: textId

        anchors.centerIn: parent
        color: Material.primaryTextColor
        elide: Text.ElideRight
        font.pixelSize: parent.height / 2
        text: `${cityName}, <i>${countryName}</i>`
        visible: !mm.locationNotSet
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
