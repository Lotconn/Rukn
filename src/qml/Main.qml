import QtQuick
import QtQuick.Controls.Material
import QtQuick.Effects
import QtQuick.Window
import Qt.labs.platform

import Munadi

ApplicationWindow {
    id: mainWindow

    LayoutMirroring.childrenInherit: true
    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    Material.accent: theme.materialAccentLight
    Material.primary: theme.materialAccentDark
    color: Material.dialogColor
    height: 770
    locale: CoreEngine.appLocale
    minimumHeight: 350
    minimumWidth: minimumHeight * theme.uiPortraitRatio
    width: 432

    background: Item {
        visible: UiEngine.bgImage

        Image {
            id: bgImg

            anchors.fill: parent
            fillMode: Image.PreserveAspectCrop
            source: "qrc:/qt/qml/Munadi/img/bg/" + (PrayerEngine.currentPrayer % 6) + ".jpg"
        }

        MultiEffect {
            anchors.fill: bgImg
            autoPaddingEnabled: false
            blur: UiEngine.blur
            blurEnabled: UiEngine.bgImage
            blurMax: 64
            source: bgImg
        }
    }

    Component.onCompleted: {
        console.log("App completion started");

        const theme = UiEngine.themeDisplayNames[UiEngine.themeDisplayIndex].name;
        mm.applyTheme(theme);

        mainWindow.height = CoreEngine.readState("Window/height", mainWindow.height);
        mainWindow.width = CoreEngine.readState("Window/width", mainWindow.width);
        mainWindow.x = CoreEngine.readState("Window/x", mainWindow.x);
        mainWindow.y = CoreEngine.readState("Window/y", mainWindow.y);

        if (engine.start_hidden())
            hide();
        else
            engine.fullscreen() ? showFullScreen() : show();

        // Only display what's new once
        if (CoreEngine.isVersionNew()) {
            whatsNewPopup.text = engine.getWhatsNew();
            whatsNewPopup.open();
        }

        if (theme === "System")
            Material.theme = Material.System;

        if (Qt.application.layoutDirection === Qt.RightToLeft)
            console.debug("Layout is" + qsTr("QT_LAYOUT_DIRECTION", "QGuiApplication"));
    }
    Component.onDestruction: {
        CoreEngine.writeState("Window/height", mainWindow.height);
        CoreEngine.writeState("Window/width", mainWindow.width);
        CoreEngine.writeState("Window/x", mainWindow.x);
        CoreEngine.writeState("Window/y", mainWindow.y, true);
    }
    onVisibilityChanged: if (mainWindow.visibility !== ApplicationWindow.Hidden && mainWindow.visibility !== ApplicationWindow.Minimized)
        CoreEngine.set60sTicker()

    Theme {
        id: theme
    }

    MunadiModel {
        id: mm
    }

    MainPage {
        id: mainPage

        property double marg: Math.max(mainWindow.width, mainWindow.height) * mm.page_margins_p

        anchors.bottomMargin: marg
        anchors.fill: parent
        anchors.leftMargin: marg
        anchors.rightMargin: marg
        anchors.topMargin: marg
    }

    // InputPanel {
    //     id: inputPanel;

    //     parent: Overlay.overlay
    //     visible: active

    //     width: parent.width
    //     y: active ? parent.height - inputPanel.height : -inputPanel.height  // If not active move it off screen, else it'll block clicks that dismiss popups!
    // }

    Popup {
        id: whatsNewPopup

        property alias text: textArea.text

        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        focus: true
        height: parent.height * 0.8
        modal: true
        parent: Overlay.overlay
        width: Math.min(parent.width, parent.height)
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2

        onClosed: {
            if (!LocationEngine.isLocationSet)
                locationPopup.open();
        }

        Flickable {
            anchors.fill: parent

            ScrollBar.vertical: ScrollBar {}
            TextArea.flickable: TextArea {
                id: textArea

                readOnly: true
                textFormat: TextArea.RichText
                wrapMode: TextArea.Wrap

                onLinkActivated: link => Qt.openUrlExternally(link)

                MouseArea {
                    acceptedButtons: Qt.NoButton
                    anchors.fill: parent
                    cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
                }
            }
        }
    }

    Popup {
        id: compassPopup

        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        focus: true
        height: parent.height * 0.8
        modal: true
        parent: Overlay.overlay
        width: Math.min(mainPage.width, 770.0)
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2

        onAboutToShow: cpLoader.sourceComponent = compassPage

        Component {
            id: compassPage

            CompassPage {
                anchors.fill: parent
            }
        }

        Loader {
            id: cpLoader

            anchors.fill: parent
        }
    }

    Popup {
        id: locationPopup

        closePolicy: LocationEngine.isLocationSet ? (Popup.CloseOnEscape | Popup.CloseOnPressOutside) : Popup.NoAutoClose
        focus: true
        modal: true
        parent: Overlay.overlay
        width: parent.width * .9
        x: (parent.width - width) / 2
        y: (parent.height - height) / 4

        onClosed: {
            if (LocationEngine.isLoading)
                LocationEngine.abortQuery();
        }

        LocationSearchComponent {
            focus: true

            Component.onCompleted: {
                forceActiveFocus();

                if (!LocationEngine.isLocationSet && !whatsNewPopup.visible)
                    locationPopup.open();
            }
            Keys.onPressed: event => {
                if (event.key === Qt.Key_Escape) {
                    if (!LocationEngine.isLocationSet && mainWindow.visibility === ApplicationWindow.FullScreen) {
                        mainWindow.visibility = ApplicationWindow.Windowed;
                        event.accepted = true;
                        return;
                    }
                }
            }
        }
    }

    Popup {
        id: settingsPopup

        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        focus: true
        height: parent.height * 0.8
        modal: true
        parent: Overlay.overlay
        width: Math.min(mainPage.width, 770.0)
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2

        onAboutToShow: spLoader.sourceComponent = settingsPage

        Component {
            id: settingsPage

            SettingsPage {
                anchors.fill: parent
            }
        }

        Loader {
            id: spLoader

            anchors.fill: parent
        }
    }

    SystemTrayIcon {
        icon.name: "org.munadi.Munadi-symbolic"
        icon.source: "qrc:/qt/qml/Munadi/img/munadi-symbolic.svg"
        tooltip: qsTr("Munadi") + ((UiEngine.timeLeftDisplay === "") ? "" : "\n" + UiEngine.timeLeftDisplay)
        visible: true

        menu: Menu {
            visible: false

            MenuItem {
                icon.source: "qrc:/qt/qml/Munadi/img/prayerTimes/fajr.svg"
                text: " " + UiEngine.prayerDisplayTimes.Fajr
            }

            MenuItem {
                icon.source: "qrc:/qt/qml/Munadi/img/prayerTimes/sunrise.svg"
                text: " " + UiEngine.prayerDisplayTimes.Sunrise
            }

            MenuItem {
                icon.source: "qrc:/qt/qml/Munadi/img/prayerTimes/dhuhr.svg"
                text: " " + UiEngine.prayerDisplayTimes.Dhuhr
            }

            MenuItem {
                icon.source: "qrc:/qt/qml/Munadi/img/prayerTimes/asr.svg"
                text: " " + UiEngine.prayerDisplayTimes.Asr
            }

            MenuItem {
                icon.source: "qrc:/qt/qml/Munadi/img/prayerTimes/maghrib.svg"
                text: " " + UiEngine.prayerDisplayTimes.Maghrib
            }

            MenuItem {
                icon.source: "qrc:/qt/qml/Munadi/img/prayerTimes/isha.svg"
                text: " " + UiEngine.prayerDisplayTimes.Isha
            }

            MenuItem {
                icon.source: "qrc:/qt/qml/Munadi/img/prayerTimes/midnight.svg"
                text: " " + UiEngine.prayerDisplayTimes.Midnight
            }

            MenuItem {
                icon.source: "qrc:/qt/qml/Munadi/img/prayerTimes/lastThirdNight.svg"
                text: " " + UiEngine.prayerDisplayTimes.LastThirdNight
            }

            MenuSeparator {}

            MenuItem {
                id: timeleft

                text: UiEngine.timeLeftDisplay + (MultimediaEngine.athanPlaying ? " 🔇" : "")
                visible: text.length > 0

                onTriggered: MultimediaEngine.athanPlaying ? MultimediaEngine.athanStop() : mainWindow.show()
            }

            MenuSeparator {}

            MenuItem {
                property bool hidden_or_min: mainWindow.visibility === ApplicationWindow.Hidden || mainWindow.visibility === ApplicationWindow.Minimized

                text: hidden_or_min ? qsTr("Show") : qsTr("Hide")

                onTriggered: hidden_or_min ? mainWindow.showNormal() : mainWindow.hide()
            }

            MenuItem {
                text: qsTr("Quit")

                onTriggered: Qt.quit()
            }
        }

        onActivated: reason => {
            switch (reason) {
            case SystemTrayIcon.Trigger:
                if (mainWindow.visible)
                    mainWindow.hide();
                else {
                    mainWindow.show();
                    mainWindow.raise();
                    mainWindow.requestActivate();
                }
            }
        }
    }

    Connections {
        function onAthanAlarm() {
            if (UiEngine.showOnAthan) {
                mainWindow.show();
                mainWindow.raise();
                mainWindow.requestActivate();
            }
        }

        function onExposeInstance() {
            mainWindow.show();
            mainWindow.raise();
            mainWindow.requestActivate();
        }

        target: CoreEngine
    }

    Connections {
        function onClosing(close) {
            if (UiEngine.hideOnClose) {
                console.debug('"Hide on close" enabled, will hide window instead of closing it.');
                close.accepted = false;
                mainWindow.hide();
            } else
                Qt.quit();
        }

        target: mainWindow
    }

    Connections {
        function onUpdateAvailable(info) {
            whatsNewPopup.text = info;
            whatsNewPopup.open();
            mainWindow.show();
            mainWindow.raise();
            mainWindow.requestActivate();
        }

        target: UpdateEngine
    }

    Shortcut {
        context: Qt.ApplicationShortcut
        sequences: [StandardKey.Quit]

        onActivated: Qt.quit()
    }

    Shortcut {
        context: Qt.WindowShortcut
        sequences: [StandardKey.MoveToNextPage]

        onActivated: ++UiEngine.dateOffset
    }

    Shortcut {
        context: Qt.WindowShortcut
        sequences: [StandardKey.MoveToPreviousPage]

        onActivated: --UiEngine.dateOffset
    }

    Shortcut {
        context: Qt.WindowShortcut
        sequences: [StandardKey.Cancel]

        onActivated: {
            if (UiEngine.dateOffset !== 0)
                UiEngine.dateOffset = undefined;
            else if (mainWindow.visibility === ApplicationWindow.FullScreen)
                mainWindow.visibility = ApplicationWindow.Windowed;
        }
    }

    MouseArea {
        acceptedButtons: Qt.BackButton
        anchors.fill: parent

        onClicked: mouse => {
            if (mouse.button === Qt.BackButton) {
                UiEngine.dateOffset = undefined;
                mouse.accepted = true;
            }
        }
    }

    Shortcut {
        sequences: [StandardKey.FullScreen]

        onActivated: {
            mainWindow.visibility = mainWindow.visibility === ApplicationWindow.FullScreen ? ApplicationWindow.Windowed : ApplicationWindow.FullScreen;
        }
    }
}
