import QtQuick 2.5
import QtQuick.Controls 2.0

import "qrc:/data/Adhan.js" as Adhan

Item {

    //visible: mm.stickerMode ? false : true
    visible: !mm.locationNotSet

    width: parent.cellWidth
    height: parent.cellHeight

    property alias text: timeLeftLabel.text

    ProgressBar {
        id: progressBar
        visible: mm.currentPrayer !== 6 && mm.nextPrayer !== 6
        anchors.bottom: parent.bottom
        anchors.bottomMargin: parent.height / 10
        anchors.horizontalCenter: parent.horizontalCenter
        from: if(mm.timeForCurrentPrayer) mm.timeForCurrentPrayer.getTime(); else 0
        to: if(mm.timeForNextPrayer) mm.timeForNextPrayer.getTime(); else 0
        value: mm.currentTime.getTime()
        width: parent.width
        opacity: 0.8
    }

    Text {
        id: timeLeftLabel
        visible: progressBar.visible || athan.isOn
        anchors.fill: parent
        anchors.bottomMargin: parent.height / 5
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        font.italic: true
        font.pixelSize: parent.height / 3
        color: theme.fontColour
        elide: Text.ElideRight
        fontSizeMode: Text.Fit

        Image {
            id: athanStopBtn
            mirror: mainPage.mirrored
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
