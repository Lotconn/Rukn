import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
// import QtCore

import Munadi

ColumnLayout {
    id: component

    TabBar {
        id: selectedPrayerIdx

        Layout.fillWidth: true
        width: parent.width

        Component.onCompleted: itemAt(1).width = 0 // Hide sunrise tab

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
            id: volText

            color: Material.secondaryTextColor
            text: qsTr("Volume")
        }

        Slider {
            id: volume

            Layout.fillWidth: true
            enabled: selectedPrayerIdx.currentIndex !== 1 // Skip sunrise tab
            stepSize: 0.05
            value: mm.prayerConfig[selectedPrayerIdx.currentIndex].volume

            onMoved: {
                mm.prayerConfig[selectedPrayerIdx.currentIndex].volume = value;
            }
        }

        Text {
            Layout.minimumWidth: volText.width
            color: Material.secondaryTextColor
            text: qsTr("%L1 %").arg(Math.round(volume.value * 100))
        }

        Text {
            color: Material.secondaryTextColor
            text: qsTr("Minutes")
        }

        Slider {
            id: mins

            Layout.fillWidth: true
            from: -30
            stepSize: 1
            to: 60
            value: mm.prayerConfig[selectedPrayerIdx.currentIndex].adjustment

            onMoved: {
                mm.prayerConfig[selectedPrayerIdx.currentIndex].adjustment = value;
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
            color: Material.secondaryTextColor
            text: mins.value > 0 ? qsTr("+%L1").arg(Math.round(mins.value)) : qsTr("%L1").arg(Math.round(mins.value))
            verticalAlignment: Text.AlignVCenter
        }

        Text {
            color: Material.secondaryTextColor
            text: qsTr("Athan")
        }

        ComboBox {
            Layout.fillWidth: true
            currentIndex: mm.prayerConfig[selectedPrayerIdx.currentIndex].athan_idx
            enabled: selectedPrayerIdx.currentIndex !== 1
            model: mm.audioList
            textRole: "name"

            onActivated: {
                mm.prayerConfig[selectedPrayerIdx.currentIndex].athan_idx = currentIndex;
            }
            onPressedChanged: model = mm.audioList
        }

        Item {}
    }
}
