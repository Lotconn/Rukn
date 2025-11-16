import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

Pane {

    Component.onCompleted: forceActiveFocus() // Hack to force focus

    Layout.preferredWidth: parent.width
    Layout.fillHeight: true

    Keys.onLeftPressed: prevDate()
    Keys.onRightPressed: nextDate()
    Keys.onDownPressed: currentDate()

    property bool buttonsVisible: true

    Material.elevation: mm.pane_elevation

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

            RoundButton {
                visible: buttonsVisible
                opacity: 0.5
                flat: true
                autoRepeat: true
                text: qsTr("<")
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

                width: parent.width
                anchors.fill: parent
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                font.weight: Font.DemiBold
                //font.pixelSize: parent.height / 1.5
                fontSizeMode: Text.Fit
                color: Material.secondaryTextColor
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
            visible: parent.columns === 3

            RoundButton {
                visible: buttonsVisible
                opacity: 0.5
                flat: true
                autoRepeat: true
                text: qsTr(">")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter

                onPressed: nextDate()
            }
        }
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
