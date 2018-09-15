import QtQuick 2.5
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1
import Qt.labs.settings 1.0
import QtPositioning 5.3

import "../data/Adhan.js" as Adhan

Page {

    PageBorder {

        Flickable {
            anchors.fill: parent
            focus: true
            clip: true
            boundsBehavior: Flickable.StopAtBounds

            Keys.onUpPressed: scrollBar.decrease()
            Keys.onDownPressed: scrollBar.increase()

            ScrollBar.vertical: ScrollBar { id: scrollBar }

            contentWidth: parent.width
            contentHeight: content.height + theme.margin * 2

            ColumnLayout {
                id: content

                spacing: 10
                width: parent.width - 40
                anchors.margins: 10

                anchors.horizontalCenter: parent.horizontalCenter

                RowLayout {} // Top padding

                RowLayout {
                    ColumnLayout {
                        SettingTitle {
                            text: qsTr("Popup on Athan?")
                        }

                        SettingSubtitle {
                            text: qsTr("Try to pop up on Athan.")
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
                            text: qsTr("Autostart?")
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

                    onAutoStartUpChanged: {
                        engine.setStartup(asu.checked);
                    }
                }

                RowLayout {
                    ColumnLayout {
                        SettingTitle {
                            text: qsTr("Volume")
                        }

                        RowLayout {
                            Slider {
                                id: vol
                                value: 1.0
                                Layout.fillWidth: true
                                onValueChanged: {
                                    athan.volume = value
                                }
                            }
                        }

                        RowLayout {
                            Text {
                                font.family: theme.defaultFontFamily; color: theme.defaultFontColour
                                text: qsTr("Mute Athan")
                                Layout.fillWidth: true
                            }
                            Switch {
                                id: muted
                                onCheckedChanged: athan.muted = muted.checked
                            }
                        }

                        Settings {
                            property alias athanVolume: vol.value
                            property alias athanMuted: muted.checked
                        }

                    }
                }

                ColumnLayout {
                    SettingTitle {
                        text: qsTr("Adjustment")
                    }

                    AdjustmentComponent {
                        id: fajrAdjustment
                        prayerId: Adhan.adhan.Prayer.Fajr

                        onValueChanged: {
                            mm.fajrAdjustment = value
                            mm.invalidate()
                        }
                    }
                    AdjustmentComponent {
                        id: sriseAdjustment
                        prayerId: Adhan.adhan.Prayer.Sunrise

                        onValueChanged: {
                            mm.sriseAdjustment = value
                            mm.invalidate()
                        }
                    }
                    AdjustmentComponent {
                        id: duhrAdjustment
                        prayerId: Adhan.adhan.Prayer.Dhuhr

                        onValueChanged: {
                            mm.duhrAdjustment = value
                            mm.invalidate()
                        }
                    }
                    AdjustmentComponent {
                        id: asrAdjustment
                        prayerId: Adhan.adhan.Prayer.Asr

                        onValueChanged: {
                            mm.asrAdjustment = value
                            mm.invalidate()
                        }
                    }
                    AdjustmentComponent {
                        id: mgrbAdjustment
                        prayerId: Adhan.adhan.Prayer.Maghrib

                        onValueChanged: {
                            mm.mgrbAdjustment = value
                            mm.invalidate()
                        }
                    }
                    AdjustmentComponent {
                        id: ishaAdjustment
                        prayerId: Adhan.adhan.Prayer.Isha

                        onValueChanged: {
                            mm.ishaAdjustment = value
                            mm.invalidate()
                        }
                    }

                    Settings {
                        category: "Adjustments"

                        property alias fajrAdjustment:      fajrAdjustment.value
                        property alias sunriseAdjustment:   sriseAdjustment.value
                        property alias duhrAdjustment:      duhrAdjustment.value
                        property alias asrAdjustment:       asrAdjustment.value
                        property alias magribAdjustment:    mgrbAdjustment.value
                        property alias ishaAdjustment:      ishaAdjustment.value
                    }
                }

                GridLayout {
                    columns: 2
                    SettingTitle {
                        text: qsTr("Other")
                        Layout.columnSpan: 2
                    }
                    Text {
                        font.family: theme.defaultFontFamily; color: theme.defaultFontColour
                        text: qsTr("Mathhab");
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
                        font.family: theme.defaultFontFamily; color: theme.defaultFontColour
                        text: qsTr("Algorithm");
                    }
                    ComboBox {
                        id: algo
                        Layout.fillWidth: true
                        currentIndex: 0

                        onActivated: {
                            mm.calcMethod = model.get(currentIndex).text
                        }

                        model: ListModel {
                            ListElement {
                                text: qsTr("Muslim World League")
                                //calcMethod: new Adhan.adhan.CalculationMethod.MuslimWorldLeague()
                            }
                            ListElement {
                                text: qsTr("Egyptian")
                                //calcMethod: new Adhan.adhan.CalculationMethod.Egyptian()
                            }
                            ListElement {
                                text: qsTr("Karachi")
                                //calcMethod: new Adhan.adhan.CalculationMethod.Karachi()
                            }
                            ListElement {
                                text: qsTr("Umm AlQura")
                                //calcMethod: new Adhan.adhan.CalculationMethod.UmmAlQura()
                            }
                            ListElement {
                                text: qsTr("Gulf")
                                //calcMethod: new Adhan.adhan.CalculationMethod.Gulf()
                            }
                            ListElement {
                                text: qsTr("Qatar")
                                //calcMethod: new Adhan.adhan.CalculationMethod.Qatar()
                            }
                            ListElement {
                                text: qsTr("Kuwait")
                                //calcMethod: new Adhan.adhan.CalculationMethod.Kuwait()
                            }
                            ListElement {
                                text: qsTr("Moonsighting Committee")
                                //calcMethod: new Adhan.adhan.CalculationMethod.MoonsightingCommittee()
                            }
                            ListElement {
                                text: qsTr("North America")
                                //calcMethod: new Adhan.adhan.CalculationMethod.NorthAmerica()
                            }
                            ListElement {
                                text: qsTr("Other")
                                //calcMethod: new Adhan.adhan.CalculationMethod.Other()
                            }
                        }
                    }

                    Text {
                        font.family: theme.defaultFontFamily; color: theme.defaultFontColour
                        text: qsTr("Calendar");
                    }
                    RowLayout {
                        ComboBox {
                            id: calendar
                            Layout.fillWidth: true

                            onCurrentIndexChanged: {
                                mm.calendarPreference = model.get(currentIndex).text
                            }

                            model: ListModel {
                                ListElement {
                                    text: qsTr("Hijri")
                                }
                                ListElement {
                                    text: qsTr("Gregorian")
                                }
                            }
                        }

                        Slider {
                            id: hijriSlider
                            Layout.fillWidth: true
                            visible: calendar.currentIndex == 0
                            from: -3; to: 3; stepSize: 1
                            snapMode: Slider.SnapAlways

                            onValueChanged: mm.hijriDateOffset = value
                        }

                        Text {
                            visible: hijriSlider.visible
                            text: hijriSlider.value >= 0 ? "+ " + hijriSlider.value.toFixed() : "- " + Math.abs(hijriSlider.value.toFixed())
                        }
                    }
                }

                Settings {
                    category: "Other"
                    property alias mathhabCbIndex: mathhab.currentIndex
                    property alias calcMethodCbIndex: algo.currentIndex
                    property alias calendarCbIndex: calendar.currentIndex
                    property alias hijriAdjustment: hijriSlider.value
                }

                RowLayout {} // Bottom padding
            }
        }
    }
}
