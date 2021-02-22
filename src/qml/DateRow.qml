import QtQuick 2.5
import QtQuick.Controls 2.0

Item {

    Component.onCompleted: forceActiveFocus() // Hack to force focus

    visible: mm.stickerMode ? false : true

    width: parent.cellWidth
    height: parent.cellHeight

    Keys.onLeftPressed: prevDate()
    Keys.onRightPressed: nextDate()
    Keys.onDownPressed: currentDate()

    Grid {
        columns: 3
        rows: 1

        anchors.fill: parent

        verticalItemAlignment: Grid.AlignVCenter
        horizontalItemAlignment: Grid.AlignHCenter

        property double colCellWidth: parent.width / columns
        property double colCellHeight: height

        Item {
            height: parent.height
            width: parent.colCellWidth / 2

            TextButton {
                enabled: false
                visible: enabled
                text: "<"
                anchors.fill: parent
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter

                onClicked: {
                    prevDate()
                }
            }
        }

        Item {
            height: parent.height
            width: parent.colCellWidth * 2

            Text {
                id: dateText
                anchors.fill: parent
                anchors.margins: 10
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                font.italic: true
                font.pixelSize: parent.height / 2
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

        Item {
            height: parent.height
            width: parent.colCellWidth / 2
            visible: parent.columns == 3

            TextButton {
                enabled: false
                visible: enabled
                text: ">"
                anchors.fill: parent
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter

                onClicked: {
                    nextDate()
                }
            }
        }
    }

    Line {
        width: parent.width - parent.width / 5
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
    }

    function currentDate() {
        mm.currentDate = new Date()
    }

    function prevDate() {
        var prevDate = mm.currentDate
        prevDate.setDate(mm.currentDate.getDate() - 1)
        mm.currentDate = prevDate
    }

    function nextDate() {
        var nextDate = mm.currentDate
        nextDate.setDate(mm.currentDate.getDate() + 1)
        mm.currentDate = nextDate
    }
}
