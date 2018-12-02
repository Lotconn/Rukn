import QtQuick 2.5
import QtQuick.Controls 2.0
import QtMultimedia 5.5

import "qrc:/data/Adhan.js" as Adhan

Item {
    width: parent.cellWidth
    height: parent.cellHeight

    property alias prayerLabel: prayerLabel.text
    property alias athanTime:   athanLabel.text
    property alias iqamaTime:   iqamaLabel.text
    property alias lineWidth:   line.width

    property var prayerId

    Grid {
        columns: iqamaLabel.text == "" ? 2 : 3;
        rows: 1

        anchors.fill: parent
        verticalItemAlignment: Grid.AlignVCenter
        horizontalItemAlignment: Grid.AlignHCenter

        property double colCellWidth: width / columns
        property double colCellHeight: height

        Item {
            height: parent.height
            width: parent.colCellWidth

            Text {
                id: prayerLabel

                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.bold: athan.isOn ? prayerId === mm.currentPrayer : prayerId === mm.nextPrayer
                font.pixelSize: Math.min(parent.height, parent.width) / 2
                color: theme.fontColour

                visible: !mm.locationNotSet
            }
        }

        Item {
            height: parent.height
            width: parent.colCellWidth

            Text {
                id: athanLabel

                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: Math.min(parent.height, parent.width) / 2
                font.bold: prayerLabel.font.bold
                color: theme.fontColour

                visible: !mm.locationNotSet
            }
        }

        Item {
            height: parent.height
            width: parent.colCellWidth
            visible: parent.columns == 3

            Text{
                id: iqamaLabel

                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: Math.min(parent.height, parent.width) / 2.5
                color: theme.fontColour
            }
        }
    }

//    Item {
//        height: parent.height; width: height
//        //width: parent.colCellWidth
//        anchors.centerIn: parent

//        Image {
//            id: athanStopBtn
//            visible: mm.currentPrayer === prayerId && athan.isOn
//            anchors.centerIn: parent
//            source: "qrc:/img/stop.png"
//            fillMode: Image.PreserveAspectFit
//            scale: 0.6
//        }

//        MouseArea {
//           id: mouseArea

//           hoverEnabled: true
//           anchors.fill: parent
//           onClicked: {
//               athan.stop()
//           }
//       }
//    }

    Line {
        id: line
        width: parent.width - parent.width / 5
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        //opacity: 0.8
    }
}
