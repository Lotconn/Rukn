import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtCore
import QtPositioning
import QtQuick.Dialogs

import "qrc:/data/Adhan.js" as Adhan

Page {
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
                    id: hoc
                    checked: false
                    onCheckedChanged: {
                        mm.hideOnClose = hoc.checked
                    }
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
                    id: soa
                    checked: false
                    onCheckedChanged: {
                        mm.showOnAthan = soa.checked
                    }
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
                    id: asu
                    checked: false
                }
                enabled: engine.ff() === "d"
                visible: enabled
            }

            //                RowLayout {
            //                    ColumnLayout {
            //                        SettingTitle {
            //                            text: qsTr("Check for updates")
            //                        }

            //                        SettingSubtitle {
            //                            text: qsTr("Munadi will check for updates when it first starts.")
            //                        }
            //                    }

            //                    Switch {
            //                        id: cfu
            //                        checked: true
            //                        onCheckedChanged: {
            //                            mm.checkForUpdates = checked
            //                        }
            //                    }
            //                    enabled: engine.ff() === "d"
            //                    visible: enabled
            //                }
            Settings {
                category: "App"
                property alias autoStartUp: asu.checked
                //property alias checkForUpdates: cfu.checked
                property alias showOnAthan: soa.checked
                property alias hideOnClose: hoc.checked

                onAutoStartUpChanged: {
                    engine.autostart(asu.checked)
                }
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
                            id: muted
                            onCheckedChanged: athan.audioOutput.muted = muted.checked
                        }
                    }

                    Settings {
                        property alias athanMuted: muted.checked
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
            }

            Settings {
                category: "Other"
                property alias mathhabCbIndex: mathhab.currentIndex
                property alias calcMethodCbIndex: algo.currentIndex
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
