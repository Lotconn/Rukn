import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtCore
import QtPositioning
import QtQuick.Dialogs

import "qrc:/qt/qml/Munadi/data/Adhan.js" as Adhan

Page {
    Component.onCompleted: {
        mathhab.currentIndex    = mm.madhab - 1
        algo.currentIndex       = mm.calcMethodIndex
        hijriSlider.value       = mm.hijriDateOffset
        calendar.currentIndex   = mm.calendarPreferenceIndex
    }

    Flickable {
        anchors.fill: parent
        focus: true
        clip: true
        boundsBehavior: Flickable.StopAtBounds



        Keys.onUpPressed: scrollBar.decrease()
        Keys.onDownPressed: scrollBar.increase()

        ScrollBar.vertical: ScrollBar {
            id: scrollBar
        }

        contentWidth: parent.width
        contentHeight: content.height + theme.margin * 2

        ColumnLayout {
            id: content

            spacing: 10
            width: parent.width - 40
            anchors.margins: 10

            anchors.horizontalCenter: parent.horizontalCenter

            RowLayout {} // Top padding

            ColumnLayout {
                width: parent.width

                Line {
                    Layout.fillWidth: true
                }

                LocationInfoRow {
                    id: locationInfoRow
                    cityName: mm.cityName
                    countryName: mm.countryName

                    Layout.preferredWidth: parent.width
                    Layout.preferredHeight: 30
                }
            }

            ColumnLayout {
                SettingTitle {
                    horizontalAlignment: Text.AlignHCenter

                    font.italic: true
                    text: qsTr("Application")
                }

                Line {
                    Layout.fillWidth: true
                }
            }

            RowLayout {
                Text {

                    color: Material.secondaryTextColor
                    text: qsTr("Theme")
                }
                ComboBox {
                    id: themeChoice
                    Layout.fillWidth: true
                    currentIndex: mm.themeListIndex

                    onActivated: {
                        mm.themeListIndex = currentIndex
                    }

                    model: mm.themeList
                }
            }

            GridLayout {
                rows: 2
                columns: 2

                Text {
                    color: Material.secondaryTextColor
                    text: qsTr("Blur")
                }
                Slider {
                    id: blur
                    Layout.fillWidth: true
                    stepSize: 1
                    value: mm.bg_img_blur
                    from: 0
                    to: 64

                    onPressedChanged: {
                        if(pressed) {
                            settingsPopup.opacity = 0
                            settingsPopup.modal = false
                        } else {
                            settingsPopup.opacity = 1
                            settingsPopup.modal = true
                        }
                    }

                    onMoved: {
                        mm.bg_img_blur = value
                    }

                }
                Text {
                    color: Material.secondaryTextColor
                    text: qsTr("Opacity")
                }
                Slider {
                    id: opacity
                    Layout.fillWidth: true
                    stepSize: .01
                    value: mm.rectangle_opacity
                    from: 0
                    to: 1

                    onPressedChanged: {
                        if(pressed) {
                            settingsPopup.opacity = 0
                            settingsPopup.modal = false
                        } else {
                            settingsPopup.opacity = 1
                            settingsPopup.modal = true
                        }
                    }

                    onMoved: {
                        mm.rectangle_opacity = value
                    }
                }
            }

            RowLayout {
                ColumnLayout {
                    SettingTitle {
                        text: qsTr("Background Images")
                    }

                    SettingSubtitle {
                        text: qsTr("Show time-of-the-day wallpapers.")
                    }
                }

                Switch {
                    checked: mm.bgImage
                    onCheckedChanged: mm.bgImage = checked
                }
                enabled: engine.ff() === "d"
                visible: enabled
            }

            RowLayout {
                ColumnLayout {
                    SettingTitle {
                        text: qsTr("Hide on close")
                    }

                    SettingSubtitle {
                        text: qsTr("When closing Munadi, hide it instead.")
                    }
                }

                Switch {
                    checked: mm.hideOnClose
                    onCheckedChanged: mm.hideOnClose = checked
                }
                enabled: engine.ff() === "d"
                visible: enabled
            }

            RowLayout {
                ColumnLayout {
                    SettingTitle {
                        text: qsTr("24-Hour Format")
                    }

                    SettingSubtitle {
                        text: qsTr("Show clock times in 24-hour format.")
                    }
                }

                Switch {
                    checked: mm.isoClock
                    onCheckedChanged: mm.isoClock = checked
                }
                enabled: engine.ff() === "d"
                visible: enabled
            }

            RowLayout {
                ColumnLayout {
                    SettingTitle {
                        text: qsTr("Show on Athan")
                    }

                    SettingSubtitle {
                        text: qsTr("Try to show window on Athan.")
                    }
                }

                Switch {
                    checked: mm.showOnAthan
                    onCheckedChanged: mm.showOnAthan = checked
                }
                enabled: engine.ff() === "d"
                visible: enabled
            }

            RowLayout {
                ColumnLayout {
                    SettingTitle {
                        text: qsTr("Autostart")
                    }

                    SettingSubtitle {
                        text: qsTr("Try to start minimised after login.")
                    }
                }

                Switch {
                    checked: mainWindow.autoStartUp
                    onCheckedChanged: {
                        if (mainWindow.autoStartUp === checked) return
                        mainWindow.autoStartUp = checked
                        engine.autostart(checked)
                    }
                }
                enabled: engine.ff() === "d"
                visible: enabled
            }

            RowLayout {
               ColumnLayout {
                   SettingTitle {
                       text: qsTr("Check for updates")
                   }

                   SettingSubtitle {
                       text: qsTr("Munadi will check for updates when it first starts.")
                   }
               }

               Switch {
                   checked: mm.checkForUpdates
                   onCheckedChanged: mm.checkForUpdates = checked
               }
               enabled: engine.ff() === "d"
               visible: enabled
            }

            SettingTitle {
                horizontalAlignment: Text.AlignHCenter
                font.italic: true
                text: qsTr("Athan")
            }
            Line {
                Layout.fillWidth: true
            }

            RowLayout {
                ColumnLayout {

                    RowLayout {
                        SettingTitle {
                            text: qsTr("Mute all Athans")
                            Layout.fillWidth: true
                        }
                        Switch {
                            checked: mm.athanMuted
                            onCheckedChanged: {
                                mm.athanMuted = checked
                                athan.audioOutput.muted = checked
                            }
                        }
                    }
                }
            }

            AudioManComp {
                id: audioManComp
                Layout.fillWidth: true
            }

            SettingTitle {
                horizontalAlignment: Text.AlignHCenter
                font.italic: true
                text: qsTr("Prayer")
            }
            Line {
                Layout.fillWidth: true
            }

            AdjustmentComponent {
                Layout.fillWidth: true
            }

            SettingTitle {
                horizontalAlignment: Text.AlignHCenter

                font.italic: true
                text: qsTr("Other")
            }
            Line {
                Layout.fillWidth: true
            }
            GridLayout {
                columns: 2
                Text {

                    color: Material.secondaryTextColor
                    text: qsTr("Mathhab")
                }
                ComboBox {
                    id: mathhab
                    Layout.fillWidth: true

                    onActivated: {
                        mm.madhab = currentIndex + 1
                    }

                    model: ListModel {
                        ListElement {
                            text: qsTr("Majority")
                        }
                        ListElement {
                            text: qsTr("Hanafi")
                        }
                    }
                }
                Text {

                    color: Material.secondaryTextColor
                    text: qsTr("Algorithm")
                }
                ComboBox {
                    id: algo
                    Layout.fillWidth: true

                    onActivated: {
                        mm.calcMethodIndex = currentIndex
                        console.debug('calcIdx: ', mm.calcMethodIndex)
                    }
                    textRole: 'name'
                    model: mm.calcMethod
                }

                Text {
                    color: Material.secondaryTextColor
                    text: qsTr("Calendar")
                }
                RowLayout {
                    ComboBox {
                        id: calendar
                        Layout.fillWidth: true

                        onActivated: {
                            mm.calendarPreferenceIndex = currentIndex
                            console.debug('calPrefIdx: ', mm.calendarPreferenceIndex)
                        }

                        textRole: 'name'
                        model: mm.calendarPreference
                    }

                    Slider {
                        id: hijriSlider
                        Layout.fillWidth: true
                        from: -4
                        to: 4
                        stepSize: 1
                        snapMode: Slider.SnapAlways

                        onValueChanged: mm.hijriDateOffset = value
                    }

                    Text {
                        color: Material.secondaryTextColor
                        text: hijriSlider.value >= 0 ? qsTr("+ %L1").arg(hijriSlider.value.toFixed())
                                                     : qsTr("- %L1").arg(Math.abs(hijriSlider.value.toFixed()))
                    }
                }
            }

            RowLayout {}

            ColumnLayout {
                SettingTitle {
                    horizontalAlignment: Text.AlignHCenter
                    font.italic: true
                    text: qsTr("About Munadi")
                }

                Line {
                    Layout.fillWidth: true
                }
            }

            GridLayout {
                columns: 2

                Text {
                    text: qsTr("Version")
                    color: Material.secondaryTextColor

                    font.italic: true
                }
                TextButton {
                    text: engine.getVersionNo()
                    onClicked: {
                        whatsNewPopup.text = engine.getWhatsNew()
                        whatsNewPopup.open()
                    }
                }
                Text {
                    text: qsTr("Website")
                    color: Material.secondaryTextColor

                    font.italic: true
                }
                TextButton {
                    property string link: "https://munadi.org"
                    text: link
                    onClicked: Qt.openUrlExternally(link)
                }
                Text {
                    text: qsTr("Contact")
                    color: Material.secondaryTextColor

                    font.italic: true
                }
                TextButton {
                    property string link: "https://munadi.org/contact"
                    text: link
                    onClicked: Qt.openUrlExternally(link)
                }
                Text {
                    text: qsTr("Support")
                    color: Material.secondaryTextColor

                    font.italic: true
                }
                TextButton {
                    property string link: "https://gitlab.com/munadi/munadi/-/issues"
                    text: link
                    Layout.fillWidth: true
                    onClicked: Qt.openUrlExternally(link)
                }
            }

            RowLayout {} // Bottom padding
        }
    }
}
