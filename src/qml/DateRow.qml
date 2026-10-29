import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

Pane {
    id: dateBox

    property bool muted: false
    readonly property var textColor: (muted) ? Material.secondaryTextColor : Material.primaryTextColor
    readonly property real textOpacity: (muted) ? 0.37 : 0.57

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
                opacity: 0.57
                text: qsTr("<")

                onPressed: --UiEngine.dateOffset
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true

            Text {
                id: dateText

                anchors.fill: parent
                color: dateBox.textColor
                elide: Text.ElideMiddle
                font.pointSize: Math.max(1, height / 2.7)
                font.weight: Font.Medium
                horizontalAlignment: Text.AlignHCenter
                opacity: dateBox.opacity
                text: UiEngine.calendarDateDisplay
                verticalAlignment: Text.AlignVCenter

                MouseArea {
                    id: mouseArea

                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true

                    onHoveredChanged: containsMouse ? dateText.font.underline = true : dateText.font.underline = false
                    onPressed: UiEngine.dateOffset = undefined
                    onWheel: wheel => (wheel.angleDelta.x < 0 || wheel.angleDelta.y > 0) ? ++UiEngine.dateOffset : --UiEngine.dateOffset
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
                opacity: 0.57
                text: qsTr(">")

                onPressed: ++UiEngine.dateOffset
            }
        }
    }
}
