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
//        Component.onCompleted: {
//            selectedPrayerTab.removeItem(1)
//        }
    }

    GridLayout {
        columns: 3
        Text {
            id: volText
            text: qsTr("Volume")
        }
        Slider {
            id: volume
            live: false
            enabled: selectedPrayerTab.currentIndex !== 1
            Layout.fillWidth: true
            value: mm.prayerConfig[selectedPrayerTab.currentIndex].volume
            //pressed always reports true, below is a work around to play sound while moving slider, then stoppting.
            onValueChanged: {
                mm.prayerConfig[selectedPrayerTab.currentIndex].volume = value
                athan.stop()
            }
            onMoved: {
                athan.volume = volume.valueAt(volume.position)
                athan.play()
            }
        }
        Text {
            //Layout.minimumWidth: parent.width * 0.1
            text: (volume.value * 100).toFixed() + qsTr("%")
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

            onValueChanged: {
                mm.prayerConfig[selectedPrayerTab.currentIndex].adjustment = value
                mm.invalidate()
            }
        }
        Text {
            verticalAlignment: Text.AlignVCenter
            font.family: theme.defaultFontFamily
            color: theme.defaultFontColour
            text: mins.value >= 0 ? "+ " + mins.value.toFixed() : "- " + Math.abs(mins.value.toFixed())
        }
    }


}
