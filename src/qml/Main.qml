import QtCore
import QtMultimedia
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Effects
import QtQuick.Window
import Qt.labs.platform

import Munadi

ApplicationWindow {
    id: mainWindow

    property bool autoStartUp: false
    property string newsVersion: ""

    function settingsBool(key, defaultValue) {
        var val = settings.value(key, defaultValue);
        return typeof val === "boolean" ? val : String(val).toLowerCase() === "true";
    }

    LayoutMirroring.childrenInherit: true
    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    Material.accent: theme.materialAccentLight
    Material.primary: theme.materialAccentDark
    color: Material.dialogColor
    height: 768
    minimumHeight: 350
    minimumWidth: minimumHeight * theme.uiPortraitRatio
    width: 432

    background: Item {
        id: todw

        visible: mm.bgImage

        Image {
            id: bgImg

            anchors.fill: parent
            fillMode: Image.PreserveAspectCrop
            mirror: Qt.application.layoutDirection === Qt.RightToLeft
            source: "qrc:/qt/qml/Munadi/img/bg/" + (mm.currentPrayer % 6) + ".jpg"
        }

        MultiEffect {
            anchors.fill: bgImg
            autoPaddingEnabled: false
            blur: 1.0
            blurEnabled: mm.bgImage
            blurMax: mm.bg_img_blur
            source: bgImg
        }
    }

    Component.onCompleted: {
        console.log("App completion started");
        settings.category = "App";
        // [BUG]: values are parsed as strings, not booleans
        // handle booleans with helper function
        autoStartUp = settingsBool("autoStartUp", false);
        mm.bgImage = settingsBool("bgImage", mm.bgImage);
        mm.checkForUpdates = settingsBool("checkForUpdates", mm.checkForUpdates);
        mm.hideOnClose = settingsBool("hideOnClose", mm.hideOnClose);
        mm.isoClock = settingsBool("isoClock", mm.isoClock);
        mm.showOnAthan = settingsBool("showOnAthan", mm.showOnAthan);

        mm.bg_img_blur = settings.value("blur", mm.bg_img_blur);
        mm.rectangle_opacity = settings.value("opacity", mm.rectangle_opacity);
        mm.themeListIndex = settings.value("themeListIndex", mm.themeListIndex);
        newsVersion = settings.value("newsVersion", "");

        settings.category = "Adhan";
        var audioList = settings.value("audioList", JSON.stringify(CoreEngine.audioList));
        try {
            CoreEngine.audioList = JSON.parse(audioList);
        } catch (e) {
            console.warn("Munadi: failed to parse stored audioList, keeping defaults:", e.message);
        }
        mm.athanMuted = settingsBool("athanMuted", mm.athanMuted);
        mm.calcMethodIndex = settings.value("calcMethodIndex", mm.calcMethodIndex);
        mm.calendarPreferenceIndex = settings.value("calendarPreferenceIndex", mm.calendarPreferenceIndex);
        mm.cityName = settings.value("cityName", mm.cityName);
        mm.countryName = settings.value("countryName", mm.countryName);
        mm.hijriDateOffset = settings.value("hijriDateOffset", mm.hijriDateOffset);
        mm.latitude = settings.value("latitude", mm.latitude);
        mm.longitude = settings.value("longitude", mm.longitude);
        mm.madhab = settings.value("madhab", mm.madhab);
        mm.overrideOffsetBy = settings.value("overrideOffsetBy", mm.overrideOffsetBy);
        var prayerConfig = settings.value("prayerConfig", JSON.stringify(CoreEngine.prayerConfig));
        try {
            CoreEngine.prayerConfig = JSON.parse(prayerConfig);
        } catch (e) {
            console.warn("Munadi: failed to parse stored prayerConfig, keeping defaults:", e.message);
        }

        settings.category = "Window";
        mainWindow.width = settings.value("width", mainWindow.width);
        mainWindow.height = settings.value("height", mainWindow.height);
        mainWindow.x = settings.value("x", mainWindow.x);
        mainWindow.y = settings.value("y", mainWindow.y);

        mm.invalidate();

        if (engine.autostart_hidden()) {
            hide();
        } else {
            engine.fullscreen() ? showFullScreen() : show();
        }

        if (mm.checkForUpdates) {
            updater.check();
        }

        // Only display what's new once
        if (newsVersion !== engine.getVersionNo()) {
            whatsNewPopup.text = engine.getWhatsNew();
            whatsNewPopup.open();
            newsVersion = engine.getVersionNo();
        }
        mm.set_24h_ticker();
        mm.set_60s_ticker();
        mm.refreshTable();

        if (mm.themeListIndex === 0) {
            mm.applySystemTheme();
        }

        if (Qt.application.layoutDirection === Qt.RightToLeft)
            console.debug("Layout is" + qsTr("QT_LAYOUT_DIRECTION", "QGuiApplication"));
    }
    Component.onDestruction: {
        settings.category = "App";
        settings.setValue("autoStartUp", autoStartUp);
        settings.setValue("bgImage", mm.bgImage);
        settings.setValue("blur", mm.bg_img_blur);
        settings.setValue("hideOnClose", mm.hideOnClose);
        settings.setValue("isoClock", mm.isoClock);
        settings.setValue("newsVersion", newsVersion);
        settings.setValue("opacity", mm.rectangle_opacity);
        settings.setValue("showOnAthan", mm.showOnAthan);
        settings.setValue("themeListIndex", mm.themeListIndex);

        settings.category = "Adhan";
        settings.setValue("athanMuted", mm.athanMuted);
        settings.setValue("audioList", JSON.stringify(CoreEngine.audioList));
        settings.setValue("calcMethodIndex", mm.calcMethodIndex);
        settings.setValue("calendarPreferenceIndex", mm.calendarPreferenceIndex);
        settings.setValue("checkForUpdates", mm.checkForUpdates);
        settings.setValue("cityName", mm.cityName);
        settings.setValue("countryName", mm.countryName);
        settings.setValue("hijriDateOffset", mm.hijriDateOffset);
        settings.setValue("latitude", mm.latitude);
        settings.setValue("longitude", mm.longitude);
        settings.setValue("madhab", mm.madhab);
        settings.setValue("overrideOffsetBy", mm.overrideOffsetBy);
        settings.setValue("prayerConfig", JSON.stringify(CoreEngine.prayerConfig));

        settings.category = "Window";
        settings.setValue("width", mainWindow.width);
        settings.setValue("height", mainWindow.height);
        settings.setValue("x", mainWindow.x);
        settings.setValue("y", mainWindow.y);

        settings.sync();
    }
    onVisibilityChanged: {
        alarm.clear("60s_tick");

        mm.set_60s_ticker();
        if (mainWindow.visibility !== ApplicationWindow.Hidden) {
            mm.refreshTable();
        }
    }

    Theme {
        id: theme
    }

    MunadiModel {
        id: mm
    }

    MediaPlayer {
        id: athan

        property bool isOn: playbackState === MediaPlayer.PlayingState

        audioOutput: AudioOutput {}
    }

    Settings {
        id: settings
    }

    LocationService {
        id: ls
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

        onClosed: if (mm.locationNotSet)
            locationPopup.open()

        Flickable {
            id: flickable

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
        id: locationPopup

        closePolicy: mm.locationNotSet ? Popup.NoAutoClose : (Popup.CloseOnEscape | Popup.CloseOnPressOutside)
        focus: true
        modal: true
        parent: Overlay.overlay
        width: parent.width * .9
        x: (parent.width - width) / 2
        y: (parent.height - height) / 4

        LocationSearchComponent {
            focus: true

            Component.onCompleted: {
                forceActiveFocus();
                if (mm.locationNotSet && !whatsNewPopup.visible) {
                    locationPopup.open();
                }
            }
            Keys.onPressed: event => {
                if (event.key === Qt.Key_Escape) {
                    if (mm.locationNotSet && mainWindow.visibility === ApplicationWindow.FullScreen) {
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
        tooltip: qsTr("Munadi") + (mm.timeLeftStr ? "\n" + mm.timeLeftStr : "")
        visible: true

        menu: Menu {
            visible: false

            onAboutToShow: {
                mm.refreshTimeLeft();
            }

            MenuItem {
                text: '🌙    ' + mm.fajr
            }

            MenuItem {
                text: '🌄    ' + mm.srise
            }

            MenuItem {
                text: '🌞    ' + mm.duhr
            }

            MenuItem {
                text: '⛅    ' + mm.asr
            }

            MenuItem {
                text: '🌇    ' + mm.mgrb
            }

            MenuItem {
                text: '🌃    ' + mm.isha
            }

            MenuSeparator {}

            MenuItem {
                id: timeleft

                text: mm.timeLeftStr + (athan.isOn ? " 🔇" : "")
                visible: text.length > 0

                onTriggered: athan.isOn ? athan.stop() : mainWindow.show()
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
                if (mainWindow.visible) {
                    mainWindow.hide();
                } else {
                    mainWindow.show();
                    mainWindow.raise();
                    mainWindow.requestActivate();
                }
            }
        }
    }

    Connections {
        function onTrigger(id) {
            console.debug("onTrigger(", id, ")");

            switch (id) {
            case "24h_tick":
                mm.currentDate = new Date(); // Recalc
                break;
            case "60s_tick":
                mm.refreshTable();
                break;
            case "alarms_reset": // Happens when clock change or wakeup after sleep for e.g.
                mm.set_24h_ticker();
                mm.set_60s_ticker();
                mm.currentDate = new Date(); // Recalc
                break;
            case "athan":
                console.debug('athan triggered: ', new Date(), ' id: ', id);

                mm.refreshTable();

                const athan_idx = CoreEngine.prayerConfig[mm.currentPrayer].athan_idx;
                athan.source = CoreEngine.audioList[athan_idx].path;
                athan.audioOutput.volume = CoreEngine.prayerConfig[mm.currentPrayer].volume;

                console.debug("athan.audioOutput.volume:", athan.audioOutput.volume);
                console.debug("athan.audioOutput.muted:", athan.audioOutput.muted);

                if (athan.audioOutput.volume > 0 && !athan.audioOutput.muted) {
                    console.debug("Will play");
                    athan.play();
                }

                if (mm.showOnAthan) {
                    mainWindow.show();
                    mainWindow.raise();
                    mainWindow.requestActivate();
                }
                break;
            }
        }

        target: alarm
    }

    Connections {
        function onPlaybackStateChanged(state) {
            mm.refreshTimeLeft();
        }

        target: athan
    }

    Connections {
        function onReceivedMessage() {
            mainWindow.show();
            mainWindow.raise();
            mainWindow.requestActivate();
        }

        target: sapp
    }

    Connections {
        function onClosing(close) {
            if (mm.hideOnClose) {
                console.debug('"Hide on close" enabled, will hide window instead of closing it.');
                close.accepted = false;
                mainWindow.hide();
            } else {
                Qt.quit();
            }
        }

        target: mainWindow
    }

    Shortcut {
        context: Qt.ApplicationShortcut
        sequences: [StandardKey.Quit]

        onActivated: Qt.quit()
    }

    Shortcut {
        context: Qt.WindowShortcut
        sequences: [StandardKey.MoveToNextPage]

        onActivated: mm.nextDate()
    }

    Shortcut {
        context: Qt.WindowShortcut
        sequences: [StandardKey.MoveToPreviousPage]

        onActivated: mm.prevDate()
    }

    Shortcut {
        context: Qt.WindowShortcut
        sequences: [StandardKey.Cancel]

        onActivated: {
            if (!mm.isToday) {
                mm.todayDate();
            } else if (mainWindow.visibility === ApplicationWindow.FullScreen) {
                mainWindow.visibility = ApplicationWindow.Windowed;
            }
        }
    }

    MouseArea {
        acceptedButtons: Qt.BackButton
        anchors.fill: parent

        onClicked: {
            if (mouse.button === Qt.BackButton) {
                mm.todayDate();
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

    Connections {
        function onUpdateAvailable(info) {
            whatsNewPopup.text = info;
            whatsNewPopup.open();
        }

        target: updater
    }
}
