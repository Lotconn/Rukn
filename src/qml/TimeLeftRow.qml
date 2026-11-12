import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import App

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
                icon.source: "qrc:/qt/qml/App/img/settings.png"
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

                anchors.bottomMargin: parent.height / 5
                anchors.fill: parent
                anchors.topMargin: anchors.bottomMargin
                color: timeleftBox.textColor
                elide: Text.ElideRight
                font.pointSize: 72
                font.weight: Font.Medium
                fontSizeMode: Text.VerticalFit
                horizontalAlignment: Text.AlignHCenter
                minimumPointSize: 1
                opacity: timeleftBox.opacity
                verticalAlignment: Text.AlignVCenter
                visible: progressBar.visible || MultimediaEngine.adhanPlaying
            }

            ProgressBar {
                id: progressBar

                Material.accent: timeLeftLabel.color
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: timeLeftLabel.bottom
                anchors.topMargin: theme.unit
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
                icon.source: "qrc:/qt/qml/App/img/stop.png"
                opacity: 0.57
                visible: MultimediaEngine.adhanPlaying

                onClicked: MultimediaEngine.adhanStop()
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.minimumWidth: parent.height

            RoundButton {
                anchors.centerIn: parent
                flat: true
                icon.source: "qrc:/qt/qml/App/img/compass.png"
                icon.width: parent.width
                opacity: 0.57

                onClicked: compassPopup.open()
            }
        }
    }
}
