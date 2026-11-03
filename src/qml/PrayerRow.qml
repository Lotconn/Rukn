pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls.Material
import QtQuick.Effects
import QtQuick.Layouts

import Munadi

RowLayout {
    id: prayerRow

    property string athanTime
    property Component iconComponent
    property bool muted: false
    property int prayerId
    readonly property var textColor: (muted) ? Material.secondaryTextColor : Material.primaryTextColor
    readonly property real textOpacity: (muted) ? 0.67 : 0.83

    Item {
        Layout.fillHeight: true
        Layout.fillWidth: true
        Layout.horizontalStretchFactor: 1
        Layout.minimumWidth: 0
        Layout.preferredWidth: 0

        Text {
            id: prayerLabel

            anchors.bottomMargin: parent.height / 3.25
            anchors.fill: parent
            anchors.leftMargin: 20 * theme.unit
            anchors.topMargin: anchors.bottomMargin
            color: prayerRow.textColor
            elide: Text.ElideRight
            font.pointSize: 72
            font.weight: (prayerRow.prayerId === PrayerEngine.currentPrayer) ? Font.Bold : Font.Normal
            fontSizeMode: Text.VerticalFit
            horizontalAlignment: Text.AlignLeft
            minimumPointSize: 1
            opacity: prayerRow.textOpacity
            text: UiEngine.prayerDisplayNames[prayerRow.prayerId].trText
            verticalAlignment: Text.AlignVCenter
            visible: LocationEngine.isLocationSet
        }
    }

    Item {
        id: iconBox

        Layout.fillHeight: true
        Layout.fillWidth: false
        Layout.leftMargin: 10
        Layout.minimumWidth: parent.height / 2
        Layout.rightMargin: -Layout.leftMargin
        layer.enabled: true
        visible: LocationEngine.isLocationSet

        layer.effect: MultiEffect {
            brightness: 1.0
            colorization: 1.0
            colorizationColor: prayerRow.textColor
        }

        Loader {
            id: prayerIcon

            anchors.centerIn: parent
            height: width
            sourceComponent: prayerRow.iconComponent
            width: Math.min(parent.width, parent.height) / 1.5 * timeRow.scale
        }
    }

    Item {
        id: timeBox

        Layout.fillHeight: true
        Layout.fillWidth: true
        Layout.horizontalStretchFactor: 1
        Layout.minimumWidth: 0
        Layout.preferredWidth: 0

        Row {
            id: timeRow

            anchors.right: parent.right
            anchors.rightMargin: 20 * theme.unit
            anchors.verticalCenter: parent.verticalCenter
            scale: implicitWidth > 0 ? Math.min(1, Math.max(0, timeBox.width - anchors.rightMargin) / implicitWidth) : 1
            spacing: 1
            visible: LocationEngine.isLocationSet
            width: implicitWidth * scale

            Text {
                id: prayerTime

                color: prayerLabel.color
                font.pointSize: prayerLabel.fontInfo.pointSize
                font.weight: prayerLabel.fontInfo.weight
                opacity: prayerLabel.opacity
                text: prayerRow.athanTime.substring(0, 5)
            }

            Text {
                anchors.baseline: prayerTime.baseline
                color: prayerTime.color
                font.pointSize: prayerTime.fontInfo.pointSize / 2
                font.weight: prayerTime.font.weight
                opacity: prayerTime.opacity
                text: prayerRow.athanTime.substring(5)
                visible: text.length > 0
            }
        }
    }
}
