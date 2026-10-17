import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import Munadi

ColumnLayout {
    id: prayerAdj

    TabBar {
        id: prayerTab

        // for init purposes only
        property string prayer: PrayerEngine.toString(PrayerEngine.Fajr)

        Layout.fillWidth: true
        implicitWidth: parent.width

        onCurrentIndexChanged: {
            const entry = UiEngine.prayerDisplayNames[currentIndex];
            if (entry)
                prayer = entry.name;
        }

        Repeater {
            model: UiEngine.prayerDisplayNames

            onItemAdded: (index, item) => {
                if (index === PrayerEngine.Sunrise)
                    item.width = 0; // Hide sunrise tab
            }

            TabButton {
                required property var modelData

                text: modelData.trText
            }
        }
    }

    RowLayout {
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
            wheelEnabled: true

            onMoved: MultimediaEngine.modifyAthanConfig(prayerTab.prayer, {
                "volume": volumeSlider.value
            })
        }

        SpinBoxComp {
            id: volumeValue

            storeVar: volumeSlider.value

            onEdited: newValue => MultimediaEngine.modifyAthanConfig(prayerTab.prayer, {
                    "volume": newValue
                })
        }
    }

    GridLayout {
        columns: 3

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
            snapMode: Slider.SnapAlways
            stepSize: 1
            to: 60
            value: PrayerEngine.prayerConfig[prayerTab.prayer].adjustment
            wheelEnabled: true

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
