import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import App

Page {
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
                SettingTitle {
                    font.italic: true
                    horizontalAlignment: Text.AlignHCenter
                    text: qsTr("Location")
                }

                Line {
                    Layout.fillWidth: true
                }
            }

            ColumnLayout {
                implicitWidth: parent.width

                LocationInfoRow {
                    id: locationInfoRow

                    Layout.preferredWidth: parent.width
                    address: LocationEngine.location.address
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
                    text: qsTr("Language")
                }

                ComboBox {
                    id: langChoice

                    Layout.fillWidth: true
                    currentIndex: UiEngine.langDisplayIndex
                    model: UiEngine.langDisplayNames
                    textRole: "trText"

                    onActivated: UiEngine.langDisplayIndex = currentIndex
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
                    currentIndex: UiEngine.themeDisplayIndex
                    model: UiEngine.themeDisplayNames
                    textRole: "trText"

                    onActivated: {
                        UiEngine.themeDisplayIndex = currentIndex;

                        mm.applyTheme(currentValue.name);
                    }
                }
            }

            GridLayout {
                columns: 3
                rows: 2

                Text {
                    id: blurLabel

                    color: Material.secondaryTextColor
                    text: qsTr("Blur")
                }

                Slider {
                    id: blurSlider

                    Layout.fillWidth: true
                    from: 0.0
                    snapMode: Slider.SnapAlways
                    stepSize: 0.01
                    to: 1.0
                    value: UiEngine.blur
                    wheelEnabled: true

                    onMoved: UiEngine.blur = value
                    onPressedChanged: {
                        if (pressed) {
                            settingsPopup.opacity = 0.1;
                            settingsPopup.modal = false;
                        } else {
                            settingsPopup.opacity = 1;
                            settingsPopup.modal = true;
                        }
                    }
                }

                SpinBoxComp {
                    id: blurValue

                    storeVar: UiEngine.blur

                    onEdited: newValue => UiEngine.blur = newValue
                }

                Text {
                    id: opacityLabel

                    color: Material.secondaryTextColor
                    text: qsTr("Opacity")
                }

                Slider {
                    id: opacitySlider

                    Layout.fillWidth: true
                    from: 0
                    snapMode: Slider.SnapAlways
                    stepSize: 0.01
                    to: 1
                    value: UiEngine.opacity
                    wheelEnabled: true

                    onMoved: UiEngine.opacity = value
                    onPressedChanged: {
                        if (pressed) {
                            settingsPopup.opacity = 0.1;
                            settingsPopup.modal = false;
                        } else {
                            settingsPopup.opacity = 1;
                            settingsPopup.modal = true;
                        }
                    }
                }

                SpinBoxComp {
                    id: opacityValue

                    storeVar: UiEngine.opacity

                    onEdited: newValue => UiEngine.opacity = newValue
                }
            }

            RowLayout {
                enabled: UiEngine.getFormFactor() === "desktop"
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
                    checked: UiEngine.bgImage

                    onClicked: UiEngine.bgImage = checked
                }
            }

            RowLayout {
                enabled: UiEngine.getFormFactor() === "desktop"
                visible: enabled

                ColumnLayout {
                    SettingTitle {
                        text: qsTr("Hide on close")
                    }

                    SettingSubtitle {
                        text: qsTr("When closing the app, hide it instead.")
                    }
                }

                Switch {
                    checked: UiEngine.hideOnClose

                    onClicked: UiEngine.hideOnClose = checked
                }
            }

            RowLayout {
                enabled: UiEngine.getFormFactor() === "desktop"
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
                    checked: UiEngine.isoClock

                    onClicked: UiEngine.isoClock = checked
                }
            }

            RowLayout {
                enabled: UiEngine.getFormFactor() === "desktop"
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
                    checked: UiEngine.showOnAthan

                    onClicked: UiEngine.showOnAthan = checked
                }
            }

            RowLayout {
                enabled: UiEngine.getFormFactor() === "desktop"
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
                    checked: CoreEngine.autoStart

                    onClicked: CoreEngine.autoStart = checked
                }
            }

            RowLayout {
                enabled: UiEngine.getFormFactor() === "desktop"
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
                    checked: UpdateEngine.checkUpdates

                    onClicked: UpdateEngine.checkUpdates = checked
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
                    currentIndex: UiEngine.madhhabDisplayIndex
                    model: UiEngine.madhhabDisplayNames
                    textRole: "trText"

                    onActivated: UiEngine.madhhabDisplayIndex = currentIndex
                }

                Text {
                    color: Material.secondaryTextColor
                    text: qsTr("Algorithm")
                }

                ComboBox {
                    id: algo

                    Layout.fillWidth: true
                    currentIndex: UiEngine.calcMethodDisplayIndex
                    model: UiEngine.calcMethodsDisplayNames
                    textRole: 'trText'

                    onActivated: UiEngine.calcMethodDisplayIndex = currentIndex
                }

                Text {
                    color: Material.secondaryTextColor
                    text: qsTr("Calendar")
                }

                RowLayout {
                    ComboBox {
                        id: calendar

                        Layout.fillWidth: true
                        currentIndex: UiEngine.calendarDisplayIndex
                        model: UiEngine.calendarDisplayNames
                        textRole: 'trText'

                        onActivated: UiEngine.calendarDisplayIndex = currentIndex
                    }

                    Slider {
                        id: hijriSlider

                        Layout.fillWidth: true
                        from: -4
                        snapMode: Slider.SnapAlways
                        stepSize: 1
                        to: 4
                        value: UiEngine.hijriOffset
                        wheelEnabled: true

                        onMoved: UiEngine.hijriOffset = value
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
                    text: qsTr("About App")
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
                    text: CoreEngine.getAppVersion()

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
