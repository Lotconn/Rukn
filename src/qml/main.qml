import QtCore
import QtMultimedia
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Effects
import QtQuick.Window
//import QtQuick.VirtualKeyboard
import Qt.labs.platform

import "qrc:/qt/qml/Munadi/data/Adhan.js" as Adhan
import HasanEsa.FileMan

ApplicationWindow {
    id: mainWindow

    Material.accent: theme.accentLightColour
    Material.primary: theme.accentDarkColour

    height: 768
    width: 444
    minimumHeight: 350
    minimumWidth: minimumHeight * theme.uiPortraitRatio
    color: Material.dialogColor
    LayoutMirroring.childrenInherit: true

    property bool   autoStartUp:    false
    property string newsVersion:    ""

    function settingsBool(key, defaultValue) {
        var val = settings.value(key, defaultValue)
        return typeof val === "boolean" ? val : String(val).toLowerCase() === "true"
    }

    background: Item {
        id: todw
        visible: mm.bgImage

        Image {
            id: bgImg
            anchors.fill: parent
            mirror: Qt.application.layoutDirection === Qt.RightToLeft
            source: "qrc:/qt/qml/Munadi/img/bg/" + (mm.currentPrayer % 6) + ".jpg"
            fillMode: Image.PreserveAspectCrop
        }

        MultiEffect {
            source: bgImg
            anchors.fill: bgImg
            blurEnabled: mm.bgImage
            blurMax: mm.bg_img_blur
            autoPaddingEnabled: false
            blur: 1.0
        }
    }

    Component.onCompleted: {

        settings.category = "App"
        // [BUG]: values are parsed as strings, not booleans
        // handle booleans with helper function
        autoStartUp         = settingsBool("autoStartUp",       false)
        mm.bgImage          = settingsBool("bgImage",           mm.bgImage)
        mm.checkForUpdates  = settingsBool("checkForUpdates",   mm.checkForUpdates)
        mm.hideOnClose      = settingsBool("hideOnClose",       mm.hideOnClose)
        mm.isoClock         = settingsBool("isoClock",          mm.isoClock)
        mm.showOnAthan      = settingsBool("showOnAthan",       mm.showOnAthan)

        mm.bg_img_blur          = settings.value("blur",            mm.bg_img_blur)
        mm.rectangle_opacity    = settings.value("opacity",         mm.rectangle_opacity)
        mm.themeListIndex       = settings.value("themeListIndex",  mm.themeListIndex)
        newsVersion             = settings.value("newsVersion",     "")

        settings.category = "Adhan"
        var audioList           = settings.value("audioList",         JSON.stringify(mm.audioList))
        try {
            mm.audioList = JSON.parse(audioList)
        } catch(_) {}
        mm.athanMuted           = settingsBool("athanMuted",            mm.athanMuted)
        mm.calcMethodIndex      = settings.value("calcMethodIndex",     mm.calcMethodIndex)
        mm.calendarPreferenceIndex   = settings.value("calendarPreferenceIndex",  mm.calendarPreferenceIndex)
        mm.cityName             = settings.value("cityName",            mm.cityName)
        mm.countryName          = settings.value("countryName",         mm.countryName)
        mm.hijriDateOffset      = settings.value("hijriDateOffset",     mm.hijriDateOffset)
        mm.latitude             = settings.value("latitude",            mm.latitude)
        mm.longitude            = settings.value("longitude",           mm.longitude)
        mm.madhab               = settings.value("madhab",              mm.madhab)
        mm.overrideOffsetBy     = settings.value("overrideOffsetBy",    mm.overrideOffsetBy)
        var prayerConfig        = settings.value("prayerConfig",      JSON.stringify(mm.prayerConfig))
        try {
            mm.prayerConfig = JSON.parse(prayerConfig)
        } catch(_) {}

        settings.category = "Window"
        mainWindow.width    = settings.value("width",   mainWindow.width)
        mainWindow.height   = settings.value("height",  mainWindow.height)
        mainWindow.x        = settings.value("x",       mainWindow.x)
        mainWindow.y        = settings.value("y",       mainWindow.y)

        mm.invalidate()

        if (engine.autostart_hidden()) {
            hide()
        } else {
            engine.fullscreen() ? showFullScreen() : show();
        }

        if(mm.checkForUpdates) {
            updater.check()
        }

        // Only display what's new once
        if (newsVersion !== engine.getVersionNo()) {
            whatsNewPopup.text = engine.getWhatsNew()
            whatsNewPopup.open()
            newsVersion = engine.getVersionNo()
        }
        mm.set_24h_ticker()
        mm.set_60s_ticker()
        mm.refreshTable()

        if (mm.themeListIndex === 0) {
            mm.applySystemTheme()
        }

        var e = FileMan.storeHouseKeeping()
        console.debug("FileMan::storeHouseKeeping() return:", e)
    }

    Component.onDestruction: {
        settings.category = "App"
        settings.setValue("autoStartUp",    autoStartUp)
        settings.setValue("bgImage",        mm.bgImage)
        settings.setValue("blur",           mm.bg_img_blur)
        settings.setValue("hideOnClose",    mm.hideOnClose)
        settings.setValue("isoClock",       mm.isoClock)
        settings.setValue("newsVersion",    newsVersion)
        settings.setValue("opacity",        mm.rectangle_opacity)
        settings.setValue("showOnAthan",    mm.showOnAthan)
        settings.setValue("themeListIndex", mm.themeListIndex)

        settings.category = "Adhan"
        settings.setValue("athanMuted",         mm.athanMuted)
        settings.setValue("audioList",          JSON.stringify(mm.audioList))
        settings.setValue("calcMethodIndex",    mm.calcMethodIndex)
        settings.setValue("calendarPreferenceIndex",    mm.calendarPreferenceIndex)
        settings.setValue("checkForUpdates",    mm.checkForUpdates)
        settings.setValue("cityName",           mm.cityName)
        settings.setValue("countryName",        mm.countryName)
        settings.setValue("hijriDateOffset",    mm.hijriDateOffset)
        settings.setValue("latitude",           mm.latitude)
        settings.setValue("longitude",          mm.longitude)
        settings.setValue("madhab",             mm.madhab)
        settings.setValue("overrideOffsetBy",   mm.overrideOffsetBy)
        settings.setValue("prayerConfig",       JSON.stringify(mm.prayerConfig))

        settings.category = "Window"
        settings.setValue("width",  mainWindow.width)
        settings.setValue("height", mainWindow.height)
        settings.setValue("x",      mainWindow.x)
        settings.setValue("y",      mainWindow.y)

        settings.sync()
    }

    onVisibilityChanged: {
        alarm.clear("60s_tick")

        mm.set_60s_ticker()
        if(mainWindow.visibility !== ApplicationWindow.Hidden) {
            mm.refreshTable()
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
        audioOutput: AudioOutput {}
        property bool isOn: playbackState === MediaPlayer.PlayingState
    }

    Settings { id: settings }
    LocationService { id: ls }

    MainPage {
        id: mainPage

        property double marg: Math.max(mainWindow.width, mainWindow.height) * mm.page_margins_p
        anchors.topMargin: marg
        anchors.bottomMargin: marg
        anchors.leftMargin: marg
        anchors.rightMargin: marg
        anchors.fill: parent
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

        parent: Overlay.overlay
        modal: true
        focus: true

        property alias text: textArea.text

        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: Math.min(parent.width, parent.height)
        height: parent.height * 0.8

        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        onClosed:  if(mm.locationNotSet) locationPopup.open()

        Flickable {
            id: flickable
            anchors.fill: parent

            TextArea.flickable: TextArea {
                id: textArea
                readOnly: true
                wrapMode: TextArea.Wrap
                textFormat: TextArea.RichText
                onLinkActivated: (link) => Qt.openUrlExternally(link)
                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.NoButton
                    cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
                }
            }

            ScrollBar.vertical: ScrollBar {}
        }
    }

    Popup {
        id: locationPopup

        parent: Overlay.overlay
        modal: true
        focus: true

        x: (parent.width - width) / 2
        y: (parent.height - height) / 4
        width: parent.width * .9

        closePolicy: mm.locationNotSet ? Popup.NoAutoClose : (Popup.CloseOnEscape | Popup.CloseOnPressOutside)

        LocationSearchComponent {
            focus: true
            Keys.onPressed: (event) => {
                if (event.key === Qt.Key_Escape) {
                    if (mm.locationNotSet && mainWindow.visibility === ApplicationWindow.FullScreen) {
                        mainWindow.visibility = ApplicationWindow.Windowed
                        event.accepted = true
                        return
                    }
                }
            }

            Component.onCompleted: {
                forceActiveFocus()
                if (mm.locationNotSet && !whatsNewPopup.visible) {
                    locationPopup.open()
                }
            }
        }
    }

    Popup {
        id: settingsPopup

        parent: Overlay.overlay
        modal: true
        focus: true

        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: Math.min(mainPage.width, 770.0)
        height: parent.height * 0.8

        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

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
        visible: true
        icon.name: "org.munadi.Munadi-symbolic"
        icon.source: "qrc:/qt/qml/Munadi/img/munadi-symbolic.svg"
        tooltip: qsTr("Munadi") + (mm.timeLeftStr ? "\n" + mm.timeLeftStr : "")

        onActivated: (reason) => {
            switch(reason) {
                case SystemTrayIcon.Trigger:
                    if(mainWindow.visible) {
                    mainWindow.hide()
                    } else {
                        mainWindow.show()
                        mainWindow.raise()
                        mainWindow.requestActivate()
                    }
            }
        }

        menu: Menu {
            visible: false

            onAboutToShow: {
                mm.refreshTimeLeft()
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
                visible: text.length > 0
                onTriggered: athan.isOn ? athan.stop() : mainWindow.show()
                text: mm.timeLeftStr + (athan.isOn ? " 🔇" : "")
            }
            MenuSeparator {}
            MenuItem {
                property bool hidden_or_min: mainWindow.visibility === ApplicationWindow.Hidden ||
                                             mainWindow.visibility === ApplicationWindow.Minimized
                text: hidden_or_min ? qsTr("Show") : qsTr("Hide")
                onTriggered: hidden_or_min ? mainWindow.showNormal() : mainWindow.hide()
            }
            MenuItem {
                text: qsTr("Quit")
                onTriggered: Qt.quit()
            }
        }
    }

    Connections {
        target: alarm

        function onTrigger(id) {
            console.debug("onTrigger(",id,")")

            switch(id) {
            case "24h_tick":
                mm.currentDate = new Date() // Recalc
                break
            case "60s_tick":
                mm.refreshTable()
                break
            case "alarms_reset": // Happens when clock change or wakeup after sleep for e.g.
                mm.set_24h_ticker()
                mm.set_60s_ticker()
                mm.currentDate = new Date() // Recalc
                break
            case "athan":
                console.debug('athan triggered: ', new Date(), ' id: ', id)

                mm.refreshTable()

                var athan_idx = mm.prayerConfig[mm.currentPrayer].athan_idx
                athan.source = mm.audioList[athan_idx]["path"]
                athan.audioOutput.volume = mm.prayerConfig[mm.currentPrayer].volume

                console.debug("athan.audioOutput.volume:", athan.audioOutput.volume)
                console.debug("athan.audioOutput.muted:", athan.audioOutput.muted)

                if(athan.audioOutput.volume > 0 && !athan.audioOutput.muted) {
                    console.debug("Will play")
                    athan.play()
                }

                if (mm.showOnAthan) {
                    mainWindow.show()
                    mainWindow.raise()
                    mainWindow.requestActivate()
                }
                break
            }
        }
    }

    Connections {
        target: athan
        function onPlaybackStateChanged(state) {
            mm.refreshTimeLeft()
        }
    }

    Connections {
        target: sapp
        function onMessageReceived() {
            mainWindow.show()
            mainWindow.raise()
            mainWindow.requestActivate()
        }
    }

    Connections {
        target: mainWindow
        function onClosing(close) {
            if(mm.hideOnClose) {
                console.debug('"Hide on close" enabled, will hide window instead of closing it.')
                close.accepted = false
                mainWindow.hide()
            } else {
                Qt.quit()
            }
        }
    }

    Shortcut {
        sequences: [StandardKey.Quit]
        context: Qt.ApplicationShortcut
        onActivated: Qt.quit()
    }

    Shortcut {
        sequences: [StandardKey.MoveToNextPage]
        context: Qt.WindowShortcut
        onActivated: mm.nextDate()
    }

    Shortcut {
        sequences: [StandardKey.MoveToPreviousPage]
        context: Qt.WindowShortcut
        onActivated: mm.prevDate()
    }

    Shortcut {
        sequences: [StandardKey.Cancel]
        context: Qt.WindowShortcut
        onActivated: {
            if(!mm.isToday) {
                mm.todayDate()
            }
            else if (mainWindow.visibility === ApplicationWindow.FullScreen) {
                mainWindow.visibility = ApplicationWindow.Windowed
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.BackButton

        onClicked: {
            if (mouse.button === Qt.BackButton) {
                mm.todayDate()
                mouse.accepted = true
            }
        }
    }

    Shortcut {
        sequences: [StandardKey.FullScreen]
        onActivated: {
            mainWindow.visibility = mainWindow.visibility === ApplicationWindow.FullScreen
                                ? ApplicationWindow.Windowed
                                : ApplicationWindow.FullScreen
        }
    }

   Connections {
       target: updater

       function onUpdateAvailable(info) {
           whatsNewPopup.text = info
           whatsNewPopup.open()
       }
   }
}
