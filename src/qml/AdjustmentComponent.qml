import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import App

ColumnLayout {
    id: prayerAdj

    TabBar {
        id: prayerTab

        // for init purposes only
        property string prayer: PrayerEngine.toString(PrayerEngine.Prayer.Fajr)

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
                if (index === PrayerEngine.Prayer.Sunrise || index > PrayerEngine.Prayer.Isha)
                    item.width = 0; // Hide undesired tabs
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
            enabled: prayerTab.currentIndex !== PrayerEngine.Prayer.Sunrise || prayerTab.currentIndex <= PrayerEngine.Prayer.Isha
            snapMode: Slider.SnapAlways
            stepSize: 0.05
            value: MultimediaEngine.adhanConfig[prayerTab.prayer].volume
            wheelEnabled: true

            onMoved: MultimediaEngine.modifyAdhanConfig(prayerTab.prayer, {
                "volume": volumeSlider.value
            })
        }

        SpinBoxComp {
            id: volumeValue

            storeVar: volumeSlider.value

            onEdited: value => MultimediaEngine.modifyAdhanConfig(prayerTab.prayer, {
                    "volume": value
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
            enabled: prayerTab.currentIndex !== PrayerEngine.Prayer.Sunrise || prayerTab.currentIndex <= PrayerEngine.Prayer.Isha
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
            id: adhanLabel

            color: Material.secondaryTextColor
            text: qsTr("Adhan")
        }

        ComboBox {
            id: adhanComboBox

            Layout.fillWidth: true
            currentIndex: MultimediaEngine.adhanConfig[prayerTab.prayer].adhan_id
            enabled: prayerTab.currentIndex !== PrayerEngine.Prayer.Sunrise || prayerTab.currentIndex <= PrayerEngine.Prayer.Isha
            model: MultimediaEngine.adhanList
            textRole: "name"

            onActivated: MultimediaEngine.modifyAdhanConfig(prayerTab.prayer, {
                "adhan_id": adhanComboBox.currentIndex
            })
        }

        Item {}
    }
}
