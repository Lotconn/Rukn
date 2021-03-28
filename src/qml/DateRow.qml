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
                enabled: true
                visible: enabled
                autoRepeat: true
                text: "<"
                anchors.fill: parent
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter

                onPressed: prevDate()
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
                    onPressed: currentDate();
                    onWheel: {
                        if (wheel.angleDelta.x < 0 || wheel.angleDelta.y > 0)
                            nextDate()
                        else
                            prevDate()
                    }
                }
            }
        }

        Item {
            height: parent.height
            width: parent.colCellWidth / 2
            visible: parent.columns == 3

            TextButton {
                enabled: true
                visible: enabled
                autoRepeat: true
                text: ">"
                anchors.fill: parent
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter

                onPressed: nextDate()
            }
        }
    }

    Line {
        width: parent.width - parent.width / 5
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
    }

    function currentDate() {
        console.debug(new Date(), ': currentDate()')
        mm.currentDate = new Date()
    }

    function prevDate() {
        console.debug(new Date(), ': prevDate()')
        var prevDate = mm.currentDate
        prevDate.setDate(prevDate.getDate() - 1)
        mm.currentDate = prevDate
    }

    function nextDate() {
        console.debug(new Date(), ': nextDate()')
        var nextDate = mm.currentDate
        nextDate.setDate(nextDate.getDate() + 1)
        mm.currentDate = nextDate
    }
}
