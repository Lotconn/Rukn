import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

Pane {
    LayoutMirroring.childrenInherit: true
    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    Material.elevation: mm.pane_elevation

    RowLayout {
        anchors.fill: parent

        Item {
            Layout.fillHeight: true
            Layout.minimumWidth: parent.height

            RoundButton {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                autoRepeat: true
                flat: true
                opacity: 0.5
                text: qsTr("<")

                onPressed: mm.prevDate()
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true

            Text {
                id: dateText

                anchors.fill: parent
                color: Material.secondaryTextColor
                elide: Text.ElideMiddle
                font.pixelSize: height / 2
                font.weight: Font.DemiBold
                horizontalAlignment: Text.AlignHCenter
                text: mm.currentDateStr
                verticalAlignment: Text.AlignVCenter

                MouseArea {
                    id: mouseArea

                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true

                    onHoveredChanged: containsMouse ? dateText.font.underline = true : dateText.font.underline = false
                    onPressed: mm.todayDate()
                    onWheel: {
                        if (wheel.angleDelta.x < 0 || wheel.angleDelta.y > 0)
                            mm.nextDate();
                        else
                            mm.prevDate();
                    }
                }
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.minimumWidth: parent.height

            RoundButton {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                autoRepeat: true
                flat: true
                opacity: 0.5
                text: qsTr(">")

                onPressed: mm.nextDate()
            }
        }
    }
}
