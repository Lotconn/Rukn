import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtMultimedia
import QtQuick.Layouts

Item {
    Layout.preferredWidth: parent.width
    Layout.fillHeight: true

    property alias prayerLabel: prayerLabel.text
    property string athanTime
    //property alias iqamaTime: iqamaLabel.text
    property double pixelDiv: 3

    property var prayerId

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 30
        anchors.rightMargin: 40
        spacing: theme.margin

        Item {
            Layout.preferredWidth: parent.width / 2
            Layout.preferredHeight: parent.height

            Text {
                id: prayerLabel
                width: parent.width
                text: mm.prayerNames[prayerId]
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                font.bold: prayerId === mm.currentPrayer
                font.pixelSize: Math.min(parent.height, parent.width) / pixelDiv
                elide: Text.ElideRight
                //color: Material.primaryTextColor
                color: Material.secondaryTextColor
                visible: !mm.locationNotSet
                font.weight: Font.ExtraLight
            }
        }

        Item {
            Layout.preferredWidth: parent.width / 2
            Layout.preferredHeight: parent.height

            Row {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                visible: !mm.locationNotSet
                spacing: 1

                Text {
                    id: athanLabel
                    font.pixelSize: Math.min(parent.parent.height,
                                             parent.parent.width) / pixelDiv
                    font.weight: prayerLabel.font.weight
                    color: prayerLabel.color
                    text: athanTime.substring(0, 5)
                }
                Text {
                    id: ttt
                    anchors.baseline: athanLabel.baseline
                    text: athanTime.substring(5)
                    font.bold: athanLabel.font.bold
                    color: athanLabel.color
                    font.pixelSize: athanLabel.font.pixelSize / 2
                    font.weight: athanLabel.font.weight
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
