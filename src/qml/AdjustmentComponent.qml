import QtQuick 2.5
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1

RowLayout {
    property alias value: slider.value
    property alias label: prayerLabel.text
    property var prayerId

    Item {
        width: 60; height: parent.height

        Text {
            id: prayerLabel
            anchors.fill: parent
            verticalAlignment: Text.AlignVCenter
            font: theme.defaultFontFamily
            color: theme.defaultFontColour
            text: mm.mapPrayerName(prayerId)
        }
    }

    Slider {
        id: slider
        Layout.fillWidth: true
        from: -20
        to: 20
        stepSize: 1
        snapMode: Slider.SnapAlways
    }
    Item {
        width: 40; height: parent.height

        Text {
            anchors.fill: parent
            verticalAlignment: Text.AlignVCenter
            font.family: theme.defaultFontFamily
            color: theme.defaultFontColour
            text: slider.value >= 0 ? "+ " + slider.value.toFixed() : "- " + Math.abs(slider.value.toFixed())
        }
    }
}
