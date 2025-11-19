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
                icon.source: "qrc:/img/settings.png"
                onClicked: settingsPopup.open()
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignCenter

            Text {
                id: timeLeftLabel

                Layout.alignment: Qt.AlignCenter
                visible: progressBar.visible || athan.isOn
                color: Material.secondaryTextColor
                elide: Text.ElideRight
                font.weight: Font.DemiBold
                fontSizeMode: Text.Fit
            }

            ProgressBar {
                id: progressBar

                Layout.alignment: Qt.AlignCenter
                Material.accent: timeLeftLabel.color
                visible: mm.currentPrayer !== Adhan.Prayer.None && mm.nextPrayer !== Adhan.Prayer.None && !athan.isOn
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
