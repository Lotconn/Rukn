import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtCore

ColumnLayout {
    id: component

    TabBar {
        id: selectedPrayerIdx

        Layout.fillWidth: true
        width: parent.width

        Repeater {
            model: mm.prayerNames
            TabButton {
                text: modelData
            }
        }

        Component.onCompleted: itemAt(1).width = 0 // Hide sunrise tab
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
            enabled: selectedPrayerIdx.currentIndex !== 1 // Skip sunrise tab
            Layout.fillWidth: true
            value: mm.prayerConfig[selectedPrayerIdx.currentIndex].volume

            onMoved: {
                mm.prayerConfig[selectedPrayerIdx.currentIndex].volume = value
            }
        }
        Text {
            color: Material.secondaryTextColor
            text: qsTr("%L1 %").arg(Math.round(volume.value * 100))
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
            value: mm.prayerConfig[selectedPrayerIdx.currentIndex].adjustment
            from: -30
            to: 60

            onPressedChanged: {
                if(pressed) {
                    settingsPopup.opacity = 0.3
                } else {
                    settingsPopup.opacity = 1
                }
            }

            onMoved: {
                mm.prayerConfig[selectedPrayerIdx.currentIndex].adjustment = value
                mm.invalidate()
            }

        }
        Text {
            verticalAlignment: Text.AlignVCenter

            color: Material.secondaryTextColor
            text: mins.value > 0 ? qsTr("+%L1").arg(Math.round(mins.value))
                                  : qsTr("%L1").arg(Math.round(mins.value))
        }

        Text {
            text: qsTr("Athan")
            color: Material.secondaryTextColor
        }
        ComboBox {
            model: mm.audioList
            Layout.fillWidth: true
            textRole: "name"
            enabled: selectedPrayerIdx.currentIndex !== 1

            currentIndex: mm.prayerConfig[selectedPrayerIdx.currentIndex].athan_idx

            onPressedChanged: model = mm.audioList

            onActivated: {
                mm.prayerConfig[selectedPrayerIdx.currentIndex].athan_idx = currentIndex
            }
        }

        Item {}
    }
}
