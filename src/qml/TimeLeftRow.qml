import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import Munadi

Pane {
    id: timeleftBox

    property bool muted: false
    property alias text: timeLeftLabel.text
    readonly property var textColor: (muted) ? Material.secondaryTextColor : Material.primaryTextColor
    readonly property real textOpacity: (muted) ? 0.37 : 0.57

    Material.elevation: mm.pane_elevation

    RowLayout {
        anchors.fill: parent

        Item {
            Layout.fillHeight: true
            Layout.minimumWidth: parent.height

            RoundButton {
                anchors.centerIn: parent
                flat: true
                icon.source: "qrc:/qt/qml/Munadi/img/settings.png"
                icon.width: parent.width
                opacity: 0.57

                onClicked: settingsPopup.open()
            }
        }

        Item {
            Layout.alignment: Qt.AlignCenter
            Layout.fillHeight: true
            Layout.fillWidth: true

            Text {
                id: timeLeftLabel

                anchors.fill: parent
                color: timeleftBox.textColor
                elide: Text.ElideRight
                font.pointSize: Math.max(1, height / 2.7)
                font.weight: Font.Medium
                horizontalAlignment: Text.AlignHCenter
                opacity: timeleftBox.opacity
                verticalAlignment: Text.AlignVCenter
                visible: progressBar.visible || MultimediaEngine.athanPlaying
            }

            ProgressBar {
                id: progressBar

                Material.accent: timeLeftLabel.color
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: timeLeftLabel.bottom
                anchors.topMargin: 2
                opacity: timeleftBox.opacity
                value: UiEngine.timeLeftProgress
                visible: timeLeftLabel.text !== ""
                width: parent.width / 1.5
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.minimumWidth: parent.height

            RoundButton {
                anchors.centerIn: parent
                flat: true
                icon.source: "qrc:/qt/qml/Munadi/img/stop.png"
                opacity: 0.57
                visible: MultimediaEngine.athanPlaying

                onClicked: MultimediaEngine.athanStop()
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.minimumWidth: parent.height

            RoundButton {
                anchors.centerIn: parent
                flat: true
                icon.source: "qrc:/qt/qml/Munadi/img/compass.png"
                icon.width: parent.width
                opacity: 0.57

                onClicked: compassPopup.open()
            }
        }
    }
}
