import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1

Rectangle {

    //visible: mm.stickerMode ? false : true
    width: parent.cellWidth
    height: parent.cellHeight

    property alias cityName: cityName.text
    property alias countryName: countryName.text

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: height / 3

        font.pixelSize: parent.height / 4
        font.italic: true
        color: "grey" //main.defaultFontColour
        elide: Text.ElideRight
        text: qsTr("Set your location ...")
        visible: mm.locationNotSet
        font.underline: cityName.font.underline
    }
    Text {
        id: cityName
        width: parent.width
        horizontalAlignment: Text.AlignHCenter
        anchors.top: parent.top
        anchors.topMargin: height / 6

        font.pixelSize: parent.height / 3
        color: theme.fontColour
        elide: Text.ElideRight

        visible: !mm.locationNotSet
    }
    Text {
        id: countryName
        width: parent.width
        horizontalAlignment: Text.AlignHCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: height / 6

        font.pixelSize: parent.height / 4
        font.italic: true
        color: "grey" //main.defaultFontColour
        elide: Text.ElideMiddle

        visible: cityName.visible
        font.underline: cityName.font.underline
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
        onHoveredChanged: containsMouse ? cityName.font.underline
                                          = true : cityName.font.underline = false
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
