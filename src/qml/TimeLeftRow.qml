import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts


import "qrc:/data/Adhan.js" as Adhan

Pane {
    Layout.preferredWidth: parent.width
    Layout.fillHeight: true

    property alias text: timeLeftLabel.text

    Material.elevation: mm.pane_elevation

    Column {
        anchors.fill: parent

        Text {
            id: timeLeftLabel

            width: parent.width
            height: parent.height / 2
            visible: progressBar.visible || athan.isOn
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            color: Material.secondaryTextColor
            elide: Text.ElideRight
            font.weight: Font.DemiBold
        }

        ProgressBar {
            id: progressBar

            Material.accent: timeLeftLabel.color
            visible: mm.currentPrayer !== Adhan.Prayer.None && mm.nextPrayer !== Adhan.Prayer.None && !athan.isOn
            anchors.top: parent.bottom
            height: timeLeftLabel.height
            anchors.horizontalCenter: parent.horizontalCenter
            from: if(mm.timeForCurrentPrayer) mm.timeForCurrentPrayer.getTime(); else 0
            to: if(mm.timeForNextPrayer) mm.timeForNextPrayer.getTime(); else 0
            value: mm.currentTime.getTime()
            width: parent.width / 2
        }
    }

    RoundButton {
        flat: true
        opacity: 0.5
        visible: athan.isOn
        icon.source: "qrc:/img/stop.png"
        icon.height: timeLeft.height / 2
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter

        onClicked: athan.stop()
    }

    RoundButton {
        flat: true
        opacity: 0.5
        icon.source: "qrc:/img/settings.png"
        icon.height: timeLeft.height/2
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        onClicked: settingsPopup.open()
    }
}
