import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import Munadi

ColumnLayout {
    id: prayerAdj

    TabBar {
        id: prayerTab

        property string prayer: PrayerEngine.toString(PrayerEngine.Fajr)

        Layout.fillWidth: true
        width: parent.width

        Component.onCompleted: itemAt(PrayerEngine.Sunrise).width = 0 // Hide sunrise tab

        onCurrentIndexChanged: prayer = PrayerEngine.toString(prayerTab.currentIndex)

        Repeater {
            model: UiEngine.prayerDisplayNamesList

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
            enabled: prayerTab.currentIndex !== PrayerEngine.Sunrise
            snapMode: Slider.SnapAlways
            stepSize: 0.05
            value: MultimediaEngine.athanConfig[prayerTab.prayer].volume

            onMoved: MultimediaEngine.modifyAthanConfig(prayerTab.prayer, {
                "volume": volumeSlider.value
            })
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
            enabled: prayerTab.currentIndex !== PrayerEngine.Sunrise
            from: -30
            stepSize: 1
            to: 60
            value: PrayerEngine.prayerConfig[prayerTab.prayer].adjustment

            onMoved: PrayerEngine.modifyPrayerConfig(prayerTab.prayer, {
                "adjustment": minutesSlider.value
            })
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
            currentIndex: MultimediaEngine.athanConfig[prayerTab.prayer].athan_idx
            enabled: prayerTab.currentIndex !== PrayerEngine.Sunrise
            model: MultimediaEngine.athanList
            textRole: "name"

            onActivated: MultimediaEngine.modifyAthanConfig(prayerTab.prayer, {
                "athan_idx": athanComboBox.currentIndex
            })
        }

        Item {}
    }
}
