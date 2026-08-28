import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
// import QtCore

import Munadi

ColumnLayout {
    id: component

    TabBar {
        id: prayerTab

        Layout.fillWidth: true
        width: parent.width

        Component.onCompleted: itemAt(PrayerEngine.Prayer.Sunrise).width = 0 // Hide sunrise tab

        Repeater {
            model: PrayerEngine.prayerNamesList

            TabButton {
                text: modelData
            }
        }
    }

    GridLayout {
        columns: 3

        Text {
            id: volumeLabel

            color: Material.secondaryTextColor
            text: qsTr("Volume")
        }

        Slider {
            id: volumeSlider

            Layout.fillWidth: true
            enabled: prayerTab.currentIndex !== PrayerEngine.Prayer.Sunrise
            snapMode: Slider.SnapAlways
            stepSize: 0.05
            value: PrayerEngine.prayerConfig[prayerTab.currentIndex].volume

            onMoved: PrayerEngine.modifyPrayerConfig(prayerTab.currentIndex, "volume", volumeSlider.value)
        }

        Text {
            id: volumeValue

            Layout.minimumWidth: volumeLabel.width
            color: Material.secondaryTextColor
            text: qsTr("%L1 %").arg(Math.round(volumeSlider.value * 100))
        }

        Text {
            id: minutesLabel

            color: Material.secondaryTextColor
            text: qsTr("Minutes")
        }

        Slider {
            id: minutesSlider

            Layout.fillWidth: true
            enabled: prayerTab.currentIndex !== PrayerEngine.Prayer.Sunrise
            from: -30
            stepSize: 1
            to: 60
            value: PrayerEngine.prayerConfig[prayerTab.currentIndex].adjustment

            onMoved: {
                PrayerEngine.modifyPrayerConfig(prayerTab.currentIndex, "adjustment", minutesSlider.value);
                mm.invalidate();
            }
            onPressedChanged: {
                if (pressed) {
                    settingsPopup.opacity = 0.3;
                } else {
                    settingsPopup.opacity = 1;
                }
            }
        }

        Text {
            id: minutesValue

            color: Material.secondaryTextColor
            text: minutesSlider.value > 0 ? qsTr("+%L1").arg(Math.round(minutesSlider.value)) : qsTr("%L1").arg(Math.round(minutesSlider.value))
            verticalAlignment: Text.AlignVCenter
        }

        Text {
            id: athanLabel

            color: Material.secondaryTextColor
            text: qsTr("Athan")
        }

        ComboBox {
            id: athanComboBox

            Layout.fillWidth: true
            currentIndex: PrayerEngine.prayerConfig[prayerTab.currentIndex].athan_idx
            enabled: prayerTab.currentIndex !== PrayerEngine.Prayer.Sunrise
            model: PrayerEngine.audioList
            textRole: "name"

            onActivated: PrayerEngine.modifyPrayerConfig(prayerTab.currentIndex, "athan_idx", athanComboBox.currentIndex)
        }

        Item {}
    }
}
