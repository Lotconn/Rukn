import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

import Munadi

Item {
    id: prayerRow

    property string athanTime
    //property alias iqamaTime: iqamaLabel.text
    readonly property double pixelDiv: 3
    property int prayerId
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

                anchors.verticalCenter: parent.verticalCenter
                color: Material.secondaryTextColor
                elide: Text.ElideRight
                font.pixelSize: Math.min(parent.height, parent.width) / prayerRow.pixelDiv
                font.weight: (prayerRow.prayerId === PrayerEngine.currentPrayer) ? Font.DemiBold : Font.Light
                horizontalAlignment: Text.AlignLeft
                text: UiEngine.prayerDisplayNames[prayerRow.prayerId].trText
                verticalAlignment: Text.AlignVCenter
                visible: LocationEngine.isLocationSet
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
                visible: LocationEngine.isLocationSet

                Text {
                    id: prayerTime

                    color: prayerLabel.color
                    font.pixelSize: prayerLabel.font.pixelSize
                    font.weight: prayerLabel.font.weight
                    horizontalAlignment: Text.AlignRight
                    text: prayerRow.athanTime.substring(0, 5)
                    verticalAlignment: prayerLabel.verticalAlignment
                }

                Text {
                    anchors.baseline: prayerTime.baseline
                    color: prayerTime.color
                    font.pixelSize: prayerTime.font.pixelSize / 2
                    font.weight: prayerTime.font.weight
                    horizontalAlignment: prayerTime.horizontalAlignment
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
