import QtQuick 2.5
import QtQuick.Controls 2.0

Item {

    Component.onCompleted: forceActiveFocus() // Hack to force focus

    visible: mm.stickerMode ? false : true

    width: parent.cellWidth
    height: parent.cellHeight

    Grid {
        columns: 1
        rows: 1

        anchors.fill: parent

        verticalItemAlignment: Grid.AlignVCenter
        horizontalItemAlignment: Grid.AlignHCenter

        property double colCellWidth: parent.width / columns
        property double colCellHeight: height

        Item {
            height: parent.height
            width: parent.width

            Text {
                id: dateText
                anchors.fill: parent
                anchors.margins: 10
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                font.italic: true
                font.pixelSize: parent.height / 3
                color: theme.fontColour
                elide: Text.ElideRight
                fontSizeMode: Text.Fit
                text: mm.currentDateStr

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onHoveredChanged: containsMouse ? dateText.font.underline
                                                      = true : dateText.font.underline = false
                }
            }
        }
    }

    Line {
        width: parent.width - parent.width / 5
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
    }
}
