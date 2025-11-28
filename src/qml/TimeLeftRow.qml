import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts


import "qrc:/data/Adhan.js" as Adhan

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
                opacity: 0.5
                icon.width: parent.width
                icon.source: "qrc:/img/settings.png"
                onClicked: settingsPopup.open()
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignCenter

            Text {
                id: timeLeftLabel

                anchors.fill: parent
                visible: progressBar.visible || athan.isOn
                color: Material.secondaryTextColor
                elide: Text.ElideRight
                font.weight: Font.DemiBold
                font.pixelSize: height / 2

                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
            }

            ProgressBar {
                id: progressBar

                width: parent.width / 1.5

                anchors.top: timeLeftLabel.bottom
                anchors.topMargin: 2
                anchors.horizontalCenter: parent.horizontalCenter
                Material.accent: timeLeftLabel.color
                visible: mm.currentPrayer !== Adhan.Prayer.None && mm.nextPrayer !== Adhan.Prayer.None && !athan.isOn && mm.dateNavOffset < 2
                from: if(mm.timeForCurrentPrayer) mm.timeForCurrentPrayer.getTime(); else 0
                to: if(mm.timeForNextPrayer) mm.timeForNextPrayer.getTime(); else 0
                value: mm.currentTime.getTime()
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.minimumWidth: parent.height

            RoundButton {
                anchors.centerIn: parent
                flat: true
                opacity: 0.5
                visible: athan.isOn
                icon.source: "qrc:/img/stop.png"

                onClicked: athan.stop()
            }
        }
    }
}
