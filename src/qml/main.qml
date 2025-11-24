import QtQuick
import QtQuick.Window
import QtCore
import QtQuick.Controls
import QtQuick.Controls.Material
import QtMultimedia
import Qt.labs.platform
import QtQuick.Effects
//import QtQuick.VirtualKeyboard

import "qrc:/data/Adhan.js" as Adhan

ApplicationWindow {
    id: mainWindow

    Material.accent: Material.Red
    Material.primary: Material.Brown

    width: 450
    height: 800
    color: Material.dialogColor
    LayoutMirroring.childrenInherit: true

    Component.onCompleted: {

        var prayerConfig = settings.value("prayerConfig", JSON.stringify(mm.prayerConfig))
        try {
            mm.prayerConfig = JSON.parse(prayerConfig)
        } catch(_) {}

        var audioList = settings.value("audioList", JSON.stringify(mm.audioList))
        try {
            mm.audioList = JSON.parse(audioList)
        } catch(_) {}

        if (engine.autostart_hidden()) {
            hide()
        } else {
            engine.fullscreen() ? showFullScreen() : show();
        }
        //        if(mm.checkForUpdates) {
        //            updater.check()
        //        }

        // Only display what's new once
        if (news.newsVersion !== engine.getVersionNo()) {
            whatsNewPopup.text = engine.getWhatsNew()
            whatsNewPopup.open()
            news.newsVersion = engine.getVersionNo()
        }
        mm.set_24h_ticker()
    }

    Component.onDestruction: {
        settings.setValue("prayerConfig", JSON.stringify(mm.prayerConfig))
        settings.setValue("audioList", JSON.stringify(mm.audioList))
    }

    onVisibilityChanged: {
        alarms.clear("60s_tick")

        if(mainWindow.visibility === ApplicationWindow.Hidden) return

        mm.set_60s_ticker()
        mm.refreshTable()
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

    // Below used for variables worth remembering only
    Settings {
        id: settings
        property alias cityName:            mm.cityName
        property alias countryName:         mm.countryName
        property alias latitude:            mm.latitude
        property alias longitude:           mm.longitude
        property alias calcMethodIndex:     mm.calcMethodIndex
        property alias madhab:              mm.madhab
        property alias overrideOffsetBy:    mm.overrideOffsetBy
        property alias opacity:             mm.rectangle_opacity
        property alias blur:                mm.bg_img_blur
        property alias themeListIndex:      mm.themeListIndex
        property alias hijriDateOffset:     mm.hijriDateOffset
        property alias calendarPreference:  mm.calendarPreference
    }

    LocationService {
        id: ls
    }

    // Window content
    Settings {
        category: "Window"
        property alias width: mainWindow.width
        property alias height: mainWindow.height
        property alias x: mainWindow.x
        property alias y: mainWindow.y
    }

    Settings {
        category: "App"
        property alias bgImage:     mm.bgImage
        property alias hideOnClose: mm.hideOnClose
        property alias isoClock:    mm.isoClock
        property alias showOnAthan: mm.showOnAthan
    }

    Item {
        id: todw
        visible: mm.bgImage
        anchors.fill: parent

        Image {
            id: bgImg
            anchors.fill: parent
            mirror: Qt.application.layoutDirection === Qt.RightToLeft
            source: "qrc:/img/bg/" + (mm.currentPrayer % 6) + ".jpg"
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

    MainPage {
        id: mainPage

        property double marg: Math.max(mainWindow.width, mainWindow.height) * mm.page_margins_p
        anchors.topMargin: marg
        anchors.bottomMargin: anchors.topMargin
        anchors.leftMargin: marg
        anchors.rightMargin: anchors.leftMargin
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

        onClosed: locationPopup.open()

        Flickable {
            id: flickable
            anchors.fill: parent

            TextArea.flickable: TextArea {
                id: textArea
                readOnly: true
                wrapMode: TextArea.Wrap
                textFormat: TextArea.RichText
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
        width: parent.width * .9 //Math.min(parent.width * .9, parent.height / 1.8)

        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        onClosed: if(mm.locationNotSet) open()

        LocationSearchComponent {
            Component.onCompleted: {
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
        width: mainPage.width
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

    Settings {
        id: news
        category: 'App'
        property string newsVersion
    }

    SystemTrayIcon {
        visible: true
        icon.mask: true
        icon.source: "qrc:/img/munadi.png"
        tooltip: qsTr("Munadi") + (mm.timeLeftStr ? "\n" + mm.timeLeftStr : "")

        onActivated: {
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
        target: alarms

        function onTrigger(id) {
            console.log("onTrigger(",id,")")

            switch(id) {
            case "24h_tick":
                mm.currentDate = new Date() // Recalc
                break
            case "60s_tick":
                mm.refreshTable()
                break
            case "alarms_reset": // Happens when clock change or wakeup after sleep for e.g.
                mm.set_24h_ticker()
                if(visibility !== ApplicationWindow.Hidden) mm.set_60s_ticker()
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
            if (mainWindow.visibility === ApplicationWindow.FullScreen) {
                mainWindow.visibility = ApplicationWindow.Windowed
            } else mm.todayDate()
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
        sequence: StandardKey.FullScreen
        onActivated: {
            mainWindow.visibility = mainWindow.visibility === ApplicationWindow.FullScreen
                                ? ApplicationWindow.Windowed
                                : ApplicationWindow.FullScreen
        }
    }

    //    Connections {
    //        target: updater
    //        onUpdateAvailable: {
    //            whatsNewPopup.text = info
    //            whatsNewPopup.open()
    //        }
    //    }
}
