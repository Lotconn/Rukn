import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

Pane {

    Material.elevation: mm.pane_elevation

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    RowLayout {
        anchors.fill: parent

        Item {
            Layout.fillHeight: true
            Layout.minimumWidth: parent.height

            RoundButton {
                opacity: 0.5
                flat: true
                autoRepeat: true
                text: qsTr("<")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter

                onPressed: mm.prevDate()
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true

            Text {
                id: dateText

                anchors.fill: parent
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                color: Material.secondaryTextColor
                text: mm.currentDateStr
                elide: Text.ElideMiddle
                font.weight: Font.DemiBold
                font.pixelSize: height / 2

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onHoveredChanged: containsMouse ? dateText.font.underline
                                                      = true : dateText.font.underline = false
                    onPressed: mm.todayDate();
                    onWheel: {
                        if (wheel.angleDelta.x < 0 || wheel.angleDelta.y > 0)
                            mm.nextDate()
                        else
                            mm.prevDate()
                    }
                }
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.minimumWidth: parent.height

            RoundButton {
                opacity: 0.5
                flat: true
                autoRepeat: true
                text: qsTr(">")
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter

                onPressed: mm.nextDate()
            }
        }
    }
}
