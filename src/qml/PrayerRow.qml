import QtQuick 2.5
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import QtMultimedia 5.5
import QtQuick.Layouts 1.15

import "qrc:/data/Adhan.js" as Adhan

Item {
    Layout.preferredWidth: parent.width
    Layout.fillHeight: true

    property alias prayerLabel: prayerLabel.text
    property string athanTime
    property alias iqamaTime: iqamaLabel.text

    property var prayerId

    RowLayout {
        anchors.fill: parent

        Item {
            Layout.preferredWidth: parent.width/2.2
            Layout.preferredHeight: parent.height

            Text {
                id: prayerLabel

                text: mm.prayerNames[prayerId]
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.bold: prayerId === mm.currentPrayer
                font.pixelSize: Math.min(parent.height, parent.width) / 2
                color: Material.secondaryTextColor

                visible: !mm.locationNotSet
            }
        }

        Item {
            Layout.preferredWidth: parent.width/2
            Layout.preferredHeight: parent.height

            Text {
                id: athanLabel

                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: Math.min(parent.height, parent.width) / 2
                font.bold: prayerLabel.font.bold
                color: Material.secondaryTextColor
                text: athanTime.substring(5, -5)

                visible: !mm.locationNotSet

                Text {
                    anchors.bottom: parent.bottom
                    anchors.left: parent.right
                    text: athanTime.substring(5)
                    font.italic: true
                    font.bold: parent.font.bold
                    color: parent.color
                    font.pixelSize: parent.font.pixelSize / 2
                }
            }
        }

        Item {
            Layout.preferredWidth: parent.width/2
            Layout.preferredHeight: parent.height
            visible: false

            Text {
                id: iqamaLabel

                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: Math.min(parent.height, parent.width) / 2.5
                color: theme.fontColour
            }
        }
    }
}
