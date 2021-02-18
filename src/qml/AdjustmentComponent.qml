import QtQuick 2.5
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1
import Qt.labs.settings 1.0

ColumnLayout {
    id: component

    TabBar {
        id: selectedPrayerTab
        Layout.fillWidth: true
        Repeater {
            model: 6
            TabButton {
                text: mm.prayerNames[index]
            }
        }
    }

    GridLayout {
        columns: 3
        Text {
            id: volText
            text: qsTr("Volume")
        }
        Slider {
            id: volume
            stepSize: 0.05
            enabled: selectedPrayerTab.currentIndex !== 1 // Skip sunrise tab
            Layout.fillWidth: true
            value: mm.prayerConfig[selectedPrayerTab.currentIndex].volume

            onMoved: {
                mm.prayerConfig[selectedPrayerTab.currentIndex].volume = value
                mm.prayerConfig = mm.prayerConfig // Workaround for Android
            }
        }
        Text {
            //Layout.minimumWidth: parent.width * 0.1
            color: theme.fontColour
            text: qsTr("%L1 %").arg(Math.abs((volume.value * 100).toFixed()))
            Layout.minimumWidth: volText.width
        }
        Text {
            text: qsTr("Minutes")
        }
        Slider {
            id: mins
            Layout.fillWidth: true
            stepSize: 1
            value: mm.prayerConfig[selectedPrayerTab.currentIndex].adjustment
            from: -10
            to: 10

            onMoved: {
                mm.prayerConfig[selectedPrayerTab.currentIndex].adjustment = value
                mm.prayerConfig = mm.prayerConfig // Workaround for Android
                mm.invalidate()
            }
        }
        Text {
            verticalAlignment: Text.AlignVCenter

            color: theme.fontColour
            text: mins.value >= 0 ? qsTr("+ %L1").arg(Math.abs(mins.value.toFixed()))
                                  : qsTr("- %L1").arg(Math.abs(mins.value.toFixed()))
        }
    }
}
