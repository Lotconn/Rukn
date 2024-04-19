import QtQuick 2.5
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import QtQuick.Layouts 1.15


import "qrc:/data/Adhan.js" as Adhan

Pane {
    Layout.preferredWidth: parent.width
    Layout.fillHeight: true

    property alias text: timeLeftLabel.text

    Material.elevation: mm.pane_elevation

    ProgressBar {
        id: progressBar

        Material.accent: timeLeftLabel.color
        visible: mm.currentPrayer !== 6 && mm.nextPrayer !== 6
        anchors.top: parent.bottom
        anchors.bottomMargin: 0//parent.height / 20
        anchors.horizontalCenter: parent.horizontalCenter
        from: if(mm.timeForCurrentPrayer) mm.timeForCurrentPrayer.getTime(); else 0
        to: if(mm.timeForNextPrayer) mm.timeForNextPrayer.getTime(); else 0
        value: mm.currentTime.getTime()
        width: parent.width / 2
    }

    Text {
        id: timeLeftLabel
        visible: progressBar.visible || athan.isOn
        anchors.fill: parent
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        color: Material.secondaryTextColor
        elide: Text.ElideRight
        font.weight: Font.DemiBold

        Image {
            id: athanStopBtn
            visible: athan.isOn
            height: timeLeft.height / 2
            anchors.right: parent.right
            anchors.rightMargin: width
            anchors.verticalCenter: parent.verticalCenter
            source: "qrc:/img/stop.png"
            fillMode: Image.PreserveAspectFit
        }
    }

    MouseArea {
        id: mouseArea

        hoverEnabled: true
        anchors.fill: parent
        onPressed: athan.stop()
    }
}
