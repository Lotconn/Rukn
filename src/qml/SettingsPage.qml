import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import Munadi

// import QtCore
// import QtPositioning
// import QtQuick.Dialogs

// import "qrc:/qt/qml/Munadi/data/Adhan.js" as Adhan

Page {
    Component.onCompleted: {
        mathhab.currentIndex = mm.madhab - 1;
        algo.currentIndex = mm.calcMethodIndex;
        hijriSlider.value = mm.hijriDateOffset;
        calendar.currentIndex = mm.calendarPreferenceIndex;
    }

    Flickable {
        LayoutMirroring.childrenInherit: true
        LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
        anchors.fill: parent
        boundsBehavior: Flickable.StopAtBounds
        clip: true
        contentHeight: content.height + theme.margin * 2
        contentWidth: parent.width
        focus: true

        ScrollBar.vertical: ScrollBar {
            id: scrollBar
        }

        Keys.onDownPressed: scrollBar.increase()
        Keys.onUpPressed: scrollBar.decrease()

        ColumnLayout {
            id: content

            anchors.horizontalCenter: parent.horizontalCenter
            anchors.margins: 10
            spacing: 10
            width: parent.width - 40

            RowLayout {} // Top padding



            ColumnLayout {
                width: parent.width

                Line {
                    Layout.fillWidth: true
                }

                LocationInfoRow {
                    id: locationInfoRow

                    Layout.preferredHeight: 30
                    Layout.preferredWidth: parent.width
                    cityName: mm.cityName
                    countryName: mm.countryName
                }
            }

            ColumnLayout {
                SettingTitle {
                    font.italic: true
                    horizontalAlignment: Text.AlignHCenter
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
                    model: UiEngine.themeList

                    onActivated: {
                        mm.themeListIndex = currentIndex;
                    }
                }
            }

            GridLayout {
                columns: 2
                rows: 2

                Text {
                    color: Material.secondaryTextColor
                    text: qsTr("Blur")
                }

                Slider {
                    id: blur

                    Layout.fillWidth: true
                    from: 0
                    stepSize: 1
                    to: 64
                    value: mm.bg_img_blur

                    onMoved: {
                        mm.bg_img_blur = value;
                    }
                    onPressedChanged: {
                        if (pressed) {
                            settingsPopup.opacity = 0;
                            settingsPopup.modal = false;
                        } else {
                            settingsPopup.opacity = 1;
                            settingsPopup.modal = true;
                        }
                    }
                }

                Text {
                    color: Material.secondaryTextColor
                    text: qsTr("Opacity")
                }

                Slider {
                    id: opacity

                    Layout.fillWidth: true
                    from: 0
                    stepSize: .01
                    to: 1
                    value: mm.rectangle_opacity

                    onMoved: {
                        mm.rectangle_opacity = value;
                    }
                    onPressedChanged: {
                        if (pressed) {
                            settingsPopup.opacity = 0;
                            settingsPopup.modal = false;
                        } else {
                            settingsPopup.opacity = 1;
                            settingsPopup.modal = true;
                        }
                    }
                }
            }

            RowLayout {
                enabled: engine.ff() === "d"
                visible: enabled

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

                    onClicked: mm.bgImage = checked
                }
            }

            RowLayout {
                enabled: engine.ff() === "d"
                visible: enabled

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

                    onClicked: mm.hideOnClose = checked
                }
            }

            RowLayout {
                enabled: engine.ff() === "d"
                visible: enabled

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

                    onClicked: mm.isoClock = checked
                }
            }

            RowLayout {
                enabled: engine.ff() === "d"
                visible: enabled

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

                    onClicked: mm.showOnAthan = checked
                }
            }

            RowLayout {
                enabled: engine.ff() === "d"
                visible: enabled

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

                    onClicked: {
                        if (mainWindow.autoStartUp === checked)
                            return;
                        mainWindow.autoStartUp = checked;
                        engine.autostart(checked);
                    }
                }
            }

            RowLayout {
                enabled: engine.ff() === "d"
                visible: enabled

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

                    onClicked: mm.checkForUpdates = checked
                }
            }

            SettingTitle {
                font.italic: true
                horizontalAlignment: Text.AlignHCenter
                text: qsTr("Athan")
            }

            Line {
                Layout.fillWidth: true
            }

            RowLayout {
                ColumnLayout {
                    RowLayout {
                        SettingTitle {
                            Layout.fillWidth: true
                            text: qsTr("Mute all Athans")
                        }

                        Switch {
                            checked: MultimediaEngine.athanMuted

                            onClicked: MultimediaEngine.athanMuted = checked
                        }
                    }
                }
            }

            AudioManComp {
                id: audioManComp

                Layout.fillWidth: true
            }

            SettingTitle {
                font.italic: true
                horizontalAlignment: Text.AlignHCenter
                text: qsTr("Prayer")
            }

            Line {
                Layout.fillWidth: true
            }

            AdjustmentComponent {
                Layout.fillWidth: true
            }

            SettingTitle {
                font.italic: true
                horizontalAlignment: Text.AlignHCenter
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

                    model: ListModel {
                        ListElement {
                            text: qsTr("Majority")
                        }

                        ListElement {
                            text: qsTr("Hanafi")
                        }
                    }

                    onActivated: {
                        mm.madhab = currentIndex + 1;
                    }
                }

                Text {
                    color: Material.secondaryTextColor
                    text: qsTr("Algorithm")
                }

                ComboBox {
                    id: algo

                    Layout.fillWidth: true
                    model: mm.calcMethod
                    textRole: 'name'

                    onActivated: {
                        mm.calcMethodIndex = currentIndex;
                        console.debug('calcIdx: ', mm.calcMethodIndex);
                    }
                }

                Text {
                    color: Material.secondaryTextColor
                    text: qsTr("Calendar")
                }

                RowLayout {
                    ComboBox {
                        id: calendar

                        Layout.fillWidth: true
                        model: mm.calendarPreference
                        textRole: 'name'

                        onActivated: {
                            mm.calendarPreferenceIndex = currentIndex;
                            console.debug('calPrefIdx: ', mm.calendarPreferenceIndex);
                        }
                    }

                    Slider {
                        id: hijriSlider

                        Layout.fillWidth: true
                        from: -4
                        snapMode: Slider.SnapAlways
                        stepSize: 1
                        to: 4

                        onValueChanged: mm.hijriDateOffset = value
                    }

                    Text {
                        color: Material.secondaryTextColor
                        text: hijriSlider.value > 0 ? qsTr("+%L1").arg(Math.round(hijriSlider.value)) : qsTr("%L1").arg(Math.round(hijriSlider.value))
                    }
                }
            }

            RowLayout {}

            ColumnLayout {
                SettingTitle {
                    font.italic: true
                    horizontalAlignment: Text.AlignHCenter
                    text: qsTr("About Munadi")
                }

                Line {
                    Layout.fillWidth: true
                }
            }

            GridLayout {
                columns: 2

                Text {
                    color: Material.secondaryTextColor
                    font.italic: true
                    text: qsTr("Version")
                }

                TextButton {
                    text: engine.getVersionNo()

                    onClicked: {
                        whatsNewPopup.text = engine.getWhatsNew();
                        whatsNewPopup.open();
                    }
                }

                Text {
                    color: Material.secondaryTextColor
                    font.italic: true
                    text: qsTr("Website")
                }

                TextButton {
                    property string link: "https://munadi.org"

                    text: link

                    onClicked: Qt.openUrlExternally(link)
                }

                ///////////////////////////////////////////////////////////////
                // FIXME: Contact doesn't work
                ///////////////////////////////////////////////////////////////
                // Text {
                //     text: qsTr("Contact")
                //     color: Material.secondaryTextColor
                //
                //     font.italic: true
                // }
                // TextButton {
                //     property string link: "https://munadi.org/contact"
                //     text: link
                //     onClicked: Qt.openUrlExternally(link)
                // }
                ///////////////////////////////////////////////////////////////
                Text {
                    color: Material.secondaryTextColor
                    font.italic: true
                    text: qsTr("Support")
                }

                TextButton {
                    property string link: "https://gitlab.com/munadi/munadi/-/work_items"

                    Layout.fillWidth: true
                    text: link

                    onClicked: Qt.openUrlExternally(link)
                }
            }

            RowLayout {} // Bottom padding
        }
    }
}
