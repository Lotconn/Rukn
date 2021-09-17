import QtQuick 2.5
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1
import Qt.labs.settings 1.0
import QtPositioning 5.3
import QtQuick.Dialogs 1.0

import "qrc:/data/Adhan.js" as Adhan

Page {

    PageBorder {

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
                        engine.setStartup(asu.checked)
                    }
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
                                onCheckedChanged: athan.muted = muted.checked
                            }
                        }

                        Settings {
                            property alias athanMuted: muted.checked
                        }
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

                        color: theme.fontColour
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

                        color: theme.fontColour
                        text: qsTr("Algorithm")
                    }
                    ComboBox {
                        id: algo
                        Layout.fillWidth: true

                        //currentIndex: 0
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
                        color: theme.fontColour

                        font.italic: true
                    }
                    Text {
                        text: engine.getVersionNo()
                        Layout.fillWidth: true
                        color: theme.fontColour
                    }
                    Text {
                        text: qsTr("Website")
                        color: theme.fontColour

                        font.italic: true
                    }
                    Text {
                        property string link: "https://munadi.org"
                        text: `<a href=\"${link}\">${link}</a>`
                        Layout.fillWidth: true
                        onLinkActivated: Qt.openUrlExternally(link)
                    }
                    Text {
                        text: qsTr("Contact")
                        color: theme.fontColour

                        font.italic: true
                    }
                    Text {
                        property string link: "https://munadi.org/contact"
                        text: `<a href=\"${link}\">${link}</a>`
                        Layout.fillWidth: true
                        onLinkActivated: Qt.openUrlExternally(link)
                    }
                    Text {
                        text: qsTr("Support")
                        color: theme.fontColour

                        font.italic: true
                    }
                    Text {
                        property string link: "https://gitlab.com/munadi/munadi/-/issues"
                        text: `<a href=\"${link}\">${link}</a>`
                        Layout.fillWidth: true
                        onLinkActivated: Qt.openUrlExternally(link)
                    }
                }

                RowLayout {} // Bottom padding
            }
        }
    }
}
