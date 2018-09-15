import QtQuick 2.5
import QtQuick.Layouts 1.2

Rectangle {
    Layout.fillHeight: true
    Layout.fillWidth: true
    border.color: main.borderColour
    property alias iconSource: img.source
    signal click

    //radius: height / 15

    color: ma.containsMouse && engine.ff() === "d" ? "lightgrey" : "white"

    Image {
        id: img
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        anchors.margins: Math.min(parent.width / 7, parent.height / 7)
        smooth: true
    }

    MouseArea {
        id: ma
        hoverEnabled: true
        anchors.fill: parent
        onClicked: parent.click()
    }
}
