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
    property double pixelDiv: 2.5

    property var prayerId

    RowLayout {
        anchors.fill: parent

        Item {
            Layout.preferredWidth: parent.width / 2
            Layout.preferredHeight: parent.height

            Text {
                id: prayerLabel
                text: mm.prayerNames[prayerId]
                //anchors.left: parent.left
                //anchors.verticalCenter: parent.verticalCenter
                //anchors.leftMargin: -50
                anchors.centerIn: parent
                //font.bold: prayerId === mm.currentPrayer
                font.pixelSize: Math.min(parent.height, parent.width) / pixelDiv
                elide: Text.ElideRight
                //color: Material.primaryTextColor
                color: Material.secondaryTextColor

                visible: !mm.locationNotSet

                font.weight: Font.DemiBold
            }
        }

        Item {
            Layout.preferredWidth: parent.width / 2
            Layout.preferredHeight: parent.height

            Text {
                id: athanLabel

                anchors.centerIn: parent
                //x: (parent.width - width + ttt.width) / 6
                //anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: Math.min(parent.height, parent.width) / pixelDiv
                //font.bold: prayerLabel.font.bold
                font.weight: prayerLabel.font.weight
                elide: Text.ElideRight
                fontSizeMode: Text.Fit
                color: prayerLabel.color
                text: athanTime.substring(5, -5)

                visible: !mm.locationNotSet

                Text {
                    id: ttt
                    anchors.bottom: parent.bottom
                    anchors.left: parent.right
                    text: "" + athanTime.substring(5)
                    //font.italic: true
                    font.bold: parent.font.bold
                    color: parent.color
                    font.pixelSize: parent.font.pixelSize / 2
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
