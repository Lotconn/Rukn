import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.1

Item {

    //visible: mm.stickerMode ? false : true
    Layout.preferredWidth: parent.width
    Layout.fillHeight: true

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
        color: Material.secondaryTextColor
        elide: Text.ElideRight

        visible: !mm.locationNotSet

        text: `${cityName}, <i>${countryName}</i>`
    }

    Line {
        width: parent.width
        anchors.bottom: parent.bottom
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true

        cursorShape: Qt.PointingHandCursor

        onClicked: popup.open()
        onHoveredChanged: textId.font.underline = containsMouse
    }

    Popup {
        id: popup
        x: parent.x
        y: parent.height
        width: parent.width
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        LocationSearchComponent {
            Component.onCompleted: {
                if (mm.locationNotSet) {
                    popup.open()
                }
            }
        }
    }
}
