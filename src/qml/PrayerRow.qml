import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

import Munadi

Item {
    id: prayerRow

    property string athanTime
    property bool muted: false
    //property alias iqamaTime: iqamaLabel.text
    readonly property double pixelDiv: 4
    property int prayerId
    property alias prayerLabel: prayerLabel.text
    readonly property var textColor: (muted) ? Material.secondaryTextColor : Material.primaryTextColor
    readonly property real textOpacity: (muted) ? 0.61 : 0.77

    Layout.fillHeight: true
    Layout.fillWidth: true

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 30
        anchors.rightMargin: 30
        spacing: theme.margin

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true

            Text {
                id: prayerLabel

                anchors.verticalCenter: parent.verticalCenter
                color: prayerRow.textColor
                elide: Text.ElideRight
                font.pointSize: Math.max(1, Math.min(parent.height, parent.width) / prayerRow.pixelDiv)
                font.weight: (prayerRow.prayerId === PrayerEngine.currentPrayer) ? Font.Bold : Font.Normal
                horizontalAlignment: Text.AlignLeft
                opacity: prayerRow.textOpacity
                text: UiEngine.prayerDisplayNames[prayerRow.prayerId].trText
                verticalAlignment: Text.AlignVCenter
                visible: LocationEngine.isLocationSet
                width: parent.width
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true

            Row {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                spacing: 1
                visible: LocationEngine.isLocationSet

                Text {
                    id: prayerTime

                    color: prayerLabel.color
                    font.pointSize: prayerLabel.font.pointSize
                    font.weight: prayerLabel.font.weight
                    horizontalAlignment: Text.AlignRight
                    opacity: prayerLabel.opacity
                    text: prayerRow.athanTime.substring(0, 5)
                    verticalAlignment: prayerLabel.verticalAlignment
                }

                Text {
                    anchors.baseline: prayerTime.baseline
                    color: prayerTime.color
                    font.pointSize: prayerTime.font.pointSize / 2
                    font.weight: prayerTime.font.weight
                    horizontalAlignment: prayerTime.horizontalAlignment
                    opacity: prayerTime.opacity
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
