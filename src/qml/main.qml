import QtQuick 2.5
import QtQuick.Window 2.0
import Qt.labs.settings 1.0
import QtQuick.Controls 2.0
import QtPositioning 5.5
import QtLocation 5.6
import QtMultimedia 5.5
import Qt.labs.platform 1.1

import "qrc:/data/Adhan.js" as Adhan

ApplicationWindow {
    id: mainWindow

    width: 400
    height: 600
    LayoutMirroring.childrenInherit: true
    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft  // Proper way
                             || qsTr("QT_LAYOUT_DIRECTION", "QGuiApplication") === "RTL"    // Old Qt version, Snap!

    Component.onCompleted: {
        if (engine.isStartupFlag()) {
            showMinimized()
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

        var midnight = new Date()
        midnight.setHours(24,0,0,0)
        alarms.add(midnight, "24h_tick", 24*60*60)

        var min = new Date()
        min.setSeconds(60);
        alarms.add(min, "60s_tick", 60)
    }

    Component.onDestruction: {
        mm.prayerConfig = mm.prayerConfig
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

    SwipeView {
        id: pagesHolder

        anchors.fill: parent
        focus: false
        currentIndex: tabBar.currentIndex

        MainPage {
            id: mainPage
        }

        SettingsPage {
            id: settingsPage
        }
    }

    footer: TabBar {
        visible: true
        id: tabBar

        //anchors.topMargin: 20
        y: 20
        focus: false
        position: TabBar.Footer
        width: parent.width
        currentIndex: pagesHolder.currentIndex

        TabButton {
            text: qsTr("Home")
        }
        TabButton {
            text: qsTr("Settings")
        }
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
            MenuItem {
                text: mm.timeLeftStr
                onTriggered: athan.stop()
                font.italic: true
            }

            MenuSeparator {}

            MenuItem {
                text: qsTr("Quit")
                onTriggered: Qt.quit()
            }
        }
    }

    Connections {
        target: alarms

        function onTrigger(id) {

            if(id === "24h_tick") {
                mm.currentDate = new Date()
                mm.invalidate()
                return
            }

            if(id === "60s_tick") {
                mm.invalidate()
                return
            }

            if(id !== "athan") return
            console.debug('js: alarm triggered: ', new Date(), ' id: ', id)

            mm.refreshTable()
            athan.volume = mm.prayerConfig[mm.currentPrayer].volume

            if(athan.volume > 0 && !athan.muted) athan.play()

            if (mm.showOnAthan) {
                mainWindow.show()
                mainWindow.raise()
                mainWindow.requestActivate()
            }
        }
    }

    Connections {
        target: athan
        function onPlaybackStateChanged(state) {
            mm.refreshTimeLeft()
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
