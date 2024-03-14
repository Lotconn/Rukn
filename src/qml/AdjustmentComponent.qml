import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import Qt.labs.settings

ColumnLayout {
    id: component

    ComboBox {
        id: selectedPrayerCB
        Layout.fillWidth: true
        model: mm.prayerNames
    }

    GridLayout {
        columns: 3
        Text {
            id: volText
            text: qsTr("Volume")
            color: Material.secondaryTextColor
        }
        Slider {
            id: volume
            stepSize: 0.05
            enabled: selectedPrayerCB.currentIndex !== 1 // Skip sunrise tab
            Layout.fillWidth: true
            value: mm.prayerConfig[selectedPrayerCB.currentIndex].volume

            onMoved: {
                mm.prayerConfig[selectedPrayerCB.currentIndex].volume = value
                mm.prayerConfig = mm.prayerConfig // Workaround for Android
            }
        }
        Text {
            color: Material.secondaryTextColor
            text: qsTr("%L1 %").arg(Math.abs((volume.value * 100).toFixed()))
            Layout.minimumWidth: volText.width
        }
        Text {
            text: qsTr("Minutes")
            color: Material.secondaryTextColor
        }
        Slider {
            id: mins
            Layout.fillWidth: true
            stepSize: 1
            value: mm.prayerConfig[selectedPrayerCB.currentIndex].adjustment
            from: -10
            to: 30

            onPressedChanged: {
                if(pressed) {
                    settingsPopup.opacity = 0.3
                } else {
                    settingsPopup.opacity = 1
                }
            }

            onMoved: {
                mm.prayerConfig[selectedPrayerCB.currentIndex].adjustment = value
                mm.prayerConfig = mm.prayerConfig // Workaround for Android
                mm.invalidate()
            }

        }
        Text {
            verticalAlignment: Text.AlignVCenter

            color: Material.secondaryTextColor
            text: mins.value >= 0 ? qsTr("+ %L1").arg(Math.abs(mins.value.toFixed()))
                                  : qsTr("- %L1").arg(Math.abs(mins.value.toFixed()))
        }
    }
}
