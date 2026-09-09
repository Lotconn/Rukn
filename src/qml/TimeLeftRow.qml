import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import "qrc:/qt/qml/Munadi/data/Adhan.js" as Adhan
import Munadi

Pane {
    property alias text: timeLeftLabel.text

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
                opacity: 0.5

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
                color: Material.secondaryTextColor
                elide: Text.ElideRight
                font.pixelSize: height / 2
                font.weight: Font.DemiBold
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                visible: progressBar.visible || MultimediaEngine.athanPlaying
            }

            ProgressBar {
                id: progressBar

                Material.accent: timeLeftLabel.color
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: timeLeftLabel.bottom
                anchors.topMargin: 2
                from: if (mm.timeForCurrentPrayer)
                    mm.timeForCurrentPrayer.getTime()
                else
                    0
                to: if (mm.timeForNextPrayer)
                    mm.timeForNextPrayer.getTime()
                else
                    0
                value: mm.currentTime.getTime()
                visible: mm.currentPrayer !== PrayerEngine.None && mm.nextPrayer !== PrayerEngine.None && !MultimediaEngine.athanPlaying && mm.dateNavOffset === 0
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
                opacity: 0.5
                visible: MultimediaEngine.athanPlaying

                onClicked: MultimediaEngine.athanStop()
            }
        }
    }
}
