pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Material
import QtQuick.Effects
import QtQuick.Layouts

import Munadi

Item {
    id: prayerRow

    property string athanTime
    property Component iconComponent
    property bool muted: false
    //property alias iqamaTime: iqamaLabel.text
    readonly property double pixelDiv: 4
    property int prayerId
    property alias prayerLabel: prayerLabel.text
    readonly property var textColor: (muted) ? Material.secondaryTextColor : Material.primaryTextColor
    readonly property real textOpacity: (muted) ? 0.67 : 0.83

    Layout.fillHeight: true
    Layout.preferredWidth: parent.width

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 30
        anchors.rightMargin: anchors.leftMargin * 2
        spacing: theme.margin

        Item {
            Layout.preferredHeight: parent.height
            Layout.preferredWidth: parent.width / 2

            Text {
                id: prayerLabel

                anchors.left: parent.left
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
            id: iconBox

            readonly property real size: prayerLabel.implicitHeight

            Layout.alignment: Qt.AlignVCenter
            Layout.preferredHeight: parent.height
            Layout.preferredWidth: parent.width / 8
            layer.enabled: true
            visible: prayerRow.iconComponent !== null

            layer.effect: MultiEffect {
                brightness: 1.0
                colorization: 1.0
                colorizationColor: prayerRow.textColor
            }

            Loader {
                id: prayerIcon

                anchors.centerIn: parent
                scale: ((width > 0) ? iconBox.size / Math.min(width, height) : 1) / 1.5
                sourceComponent: prayerRow.iconComponent
                transformOrigin: Item.Center
            }
        }

        Item {
            Layout.preferredHeight: parent.height
            Layout.preferredWidth: parent.width / 2

            Row {
                anchors.centerIn: parent
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
