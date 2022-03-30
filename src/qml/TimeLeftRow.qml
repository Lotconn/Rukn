import QtQuick 2.5
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import QtQuick.Layouts 1.15


import "qrc:/data/Adhan.js" as Adhan

Item {

    //visible: mm.stickerMode ? false : true
    visible: !mm.locationNotSet

    Layout.preferredWidth: parent.width
    Layout.fillHeight: true

    property alias text: timeLeftLabel.text

    ProgressBar {
        id: progressBar

        Material.accent: Material.Grey
        visible: mm.currentPrayer !== 6 && mm.nextPrayer !== 6
        anchors.bottom: parent.bottom
        anchors.bottomMargin: parent.height / 6
        anchors.horizontalCenter: parent.horizontalCenter
        from: if(mm.timeForCurrentPrayer) mm.timeForCurrentPrayer.getTime(); else 0
        to: if(mm.timeForNextPrayer) mm.timeForNextPrayer.getTime(); else 0
        value: mm.currentTime.getTime()
        width: parent.width / 2
        opacity: 0.6
    }

    Text {
        id: timeLeftLabel
        visible: progressBar.visible || athan.isOn
        anchors.fill: parent
        anchors.bottomMargin: parent.height / 3
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        font.italic: true
        font.pixelSize: parent.height
        color: Material.secondaryTextColor
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
