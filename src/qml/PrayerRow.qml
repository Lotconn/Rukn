import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

import Munadi

Item {
    id: prayerRow

    property string athanTime
    //property alias iqamaTime: iqamaLabel.text
    property double pixelDiv: 3
    property var prayerId
    property alias prayerLabel: prayerLabel.text

    Layout.fillHeight: true
    Layout.preferredWidth: parent.width

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 30
        anchors.rightMargin: 40
        spacing: theme.margin

        Item {
            Layout.preferredHeight: parent.height
            Layout.preferredWidth: parent.width / 2

            Text {
                id: prayerLabel

                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                //color: Material.primaryTextColor
                color: Material.secondaryTextColor
                elide: Text.ElideRight
                font.bold: prayerRow.prayerId === mm.currentPrayer
                font.pixelSize: Math.min(parent.height, parent.width) / prayerRow.pixelDiv
                font.weight: Font.Thin
                text: UiEngine.prayerDisplayNames[prayerRow.prayerId].trText
                visible: !mm.locationNotSet
                width: parent.width
            }
        }

        Item {
            Layout.preferredHeight: parent.height
            Layout.preferredWidth: parent.width / 2

            Row {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                spacing: 1
                visible: !mm.locationNotSet

                Text {
                    id: athanLabel

                    color: prayerLabel.color
                    font.pixelSize: Math.min(parent.parent.height, parent.parent.width) / pixelDiv
                    font.weight: prayerLabel.font.weight
                    text: prayerRow.athanTime.substring(0, 5)
                }

                Text {
                    id: ttt

                    anchors.baseline: athanLabel.baseline
                    color: athanLabel.color
                    font.bold: athanLabel.font.bold
                    font.pixelSize: athanLabel.font.pixelSize / 2
                    font.weight: athanLabel.font.weight
                    text: prayerRow.athanTime.substring(5)
                }
            }
        }

        // Item {
        //     Layout.preferredWidth: parent.width / 2
        //     Layout.preferredHeight: parent.height
        //     visible: false

        //     Text {
        //         id: iqamaLabel

        //         anchors.verticalCenter: parent.verticalCenter
        //         anchors.horizontalCenter: parent.horizontalCenter
        //         font.pixelSize: Math.min(parent.height, parent.width) / pixelDiv
        //         color: theme.fontColour
        //     }
        // }
    }
}
