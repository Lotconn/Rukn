import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

Pane {

    Keys.onLeftPressed: prevDate()
    Keys.onRightPressed: nextDate()
    Keys.onDownPressed: currentDate()

    Material.elevation: mm.pane_elevation

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 20
        anchors.rightMargin: 20

        Item {
            height: parent.height

            RoundButton {
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
            Layout.fillWidth: true

            Text {
                id: dateText
                anchors.fill: parent
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                font.weight: Font.DemiBold
                fontSizeMode: Text.VerticalFit
                color: Material.secondaryTextColor
                text: mm.currentDateStr
            }
        }

        Item {
            height: parent.height

            RoundButton {
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
