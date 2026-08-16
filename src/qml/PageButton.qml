import QtQuick
import QtQuick.Layouts

Rectangle {
    property alias iconSource: img.source

    signal click

    Layout.fillHeight: true
    Layout.fillWidth: true
    border.color: main.borderColour

    //radius: height / 15
    color: ma.containsMouse && engine.ff() === "d" ? "lightgrey" : "white"

    Image {
        id: img

        anchors.fill: parent
        anchors.margins: Math.min(parent.width / 7, parent.height / 7)
        fillMode: Image.PreserveAspectFit
        smooth: true
    }

    MouseArea {
        id: ma

        anchors.fill: parent
        hoverEnabled: true

        onClicked: parent.click()
    }
}
