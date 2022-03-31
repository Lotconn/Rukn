import QtQuick 2.5
import QtQuick.Window 2.0
import Qt.labs.settings 1.0
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0
import QtPositioning 5.5
import QtLocation 5.6
import QtMultimedia 5.5
import Qt.labs.platform 1.1

import "qrc:/data/Adhan.js" as Adhan

ApplicationWindow {
    id: mainWindow

    Material.theme: Material.System
    Material.accent: Material.BlueGrey

    width: 400
    height: 600
    color: Material.dialogColor
    LayoutMirroring.childrenInherit: true
    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft  // Proper way
                             || qsTr("QT_LAYOUT_DIRECTION", "QGuiApplication") === "RTL"    // Old Qt version, Snap!

    Component.onCompleted: {
        if (engine.autostart_hidden()) {
            hide()
        } else {
            show()
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
        mm.prayerConfig = mm.prayerConfig
    }

    onVisibilityChanged: {
        alarms.clear("60s_tick")

        if(visibility === ApplicationWindow.Hidden) return

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
        property bool isOn: playbackState === MediaPlayer.PlayingState 
    }

    // Below used for variables worth remembering only
    Settings {
        property alias cityName:            mm.cityName
        property alias countryName:         mm.countryName
        property alias latitude:            mm.latitude
        property alias longitude:           mm.longitude
        property alias calcMethodIndex:     mm.calcMethodIndex
        property alias madhab:              mm.madhab
        property alias overrideOffsetBy:    mm.overrideOffsetBy
        property alias prayerConfig:        mm.prayerConfig
        property alias audioList:           mm.audioList
        property alias audioListIndex:      mm.audioListIndex
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

    MainPage {
        id: mainPage
        anchors.margins: 10
        anchors.fill: parent
    }

    Popup {
        id: whatsNewPopup

        parent: ApplicationWindow.overlay
        modal: true

        property alias text: textArea.text

        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: parent.width * 0.8
        height: parent.height * 0.8

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
        id: settingsPopup

        parent: ApplicationWindow.overlay
        modal: true

        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: parent.width * 0.9
        height: parent.height * 0.8

        Flickable {
            anchors.fill: parent

            SettingsPage {
                anchors.fill: parent
            }

            ScrollBar.vertical: ScrollBar {}
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
                athan.volume = mm.prayerConfig[mm.currentPrayer].volume

                if(athan.volume > 0 && !athan.muted) {
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
            }
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
