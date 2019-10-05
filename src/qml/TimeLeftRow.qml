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
        anchors.bottom: parent.bottom
        anchors.bottomMargin: parent.height / 10
        anchors.horizontalCenter: parent.horizontalCenter
        from: mm.timeForCurrentPrayer ? mm.timeForCurrentPrayer.getTime() : 0
        to: mm.timeForNextPrayer ? mm.timeForNextPrayer.getTime() : 0    // Next prayer might be None, so calc next day Fajr!
        value: mm.currentDate.getTime()
        width: parent.width
        opacity: 0.8
    }

    Text {
        id: timeLeftLabel
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
       onClicked: {
           athan.stop()
       }
   }
}
