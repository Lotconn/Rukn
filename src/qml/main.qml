import QtQuick 2.5
import QtQuick.Window 2.0
import Qt.labs.settings 1.0
import QtQuick.Controls 2.0
import QtPositioning 5.5
import QtLocation 5.6
import QtMultimedia 5.5

import "qrc:/data/Adhan.js" as Adhan

ApplicationWindow {
    id: mainWindow

    width: 400; height: 600

    Component.onCompleted: {

        if(engine.isStartupFlag()) {
            showMinimized()
        } else {
            show()
        }

//        if(mm.checkForUpdates) {
//            updater.check()
//        }

        // Only display what's new once
        if(news.newsVersion !== engine.getVersionNo()) {
            whatsNewPopup.text = engine.getWhatsNew()
            whatsNewPopup.open()
            news.newsVersion = engine.getVersionNo();
        }
    }

    Component.onDestruction: {
        mm.prayerConfig = mm.prayerConfig
    }

    // Globals

    MunadiModel {
        id: mm
    }

    MediaPlayer {
        id: athan
        source: "qrc:/data/athan.ogg"
        property bool isOn: playbackState === MediaPlayer.PlayingState
    }

    Timer {
        interval: 1000; running: true; repeat: true

        onTriggered: {
            var date = new Date()
            mm.currentDate = date

            if(date.getSeconds() == 0) {

                if(mm.nextPrayer !== Adhan.adhan.Prayer.None
                        && mm.nextPrayer !== Adhan.adhan.Prayer.Sunrise
                        && mm.timeForNextPrayer.getHours() === date.getHours()
                        && mm.timeForNextPrayer.getMinutes() === date.getMinutes()) {

                    athan.volume = mm.prayerConfig[mm.currentPrayer].volume
                    athan.play()

                    if(mm.showOnAthan) {
                        mainWindow.show()
                        mainWindow.raise()
                        mainWindow.requestActivate()
                    }
                }

                mm.currentDate = date
            }

            if(athan.isOn) {
                mm.timeLeftStr = mm.prayerNames[mm.currentPrayer] + qsTr(" Athan ...")
            } else {
                if(mm.nextPrayer === Adhan.adhan.Prayer.None) {
                    mm.timeLeftStr = ""
                } else {
                    var totalMins = (mm.timeForNextPrayer.getTime() - mm.currentDate.getTime()) / 60000
                    var hrs = Math.floor(totalMins / 60)
                    var mins = Math.ceil(totalMins % 60)

                    mm.timeLeftStr = hrs + qsTr("h ") + mins + qsTr("m until ") + mm.prayerNames[mm.nextPrayer]
                }
            }
        }
    }

    // Below used for variables worth remembering only

    Settings {
        property alias cityName:            mm.cityName
        property alias stateName:           mm.stateName
        property alias countryName:         mm.countryName
        property alias timeZone:            mm.timeZone
        property alias latitude:            mm.latitude
        property alias longitude:           mm.longitude
        property alias calcMethod:          mm.calcMethod
        property alias madhab:              mm.madhab
        property alias overrideOffsetBy:    mm.overrideOffsetBy
        property alias prayerConfig:        mm.prayerConfig
    }

    LocationService {
        id: ls
    }

    Theme {
        id: theme
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
        interactive: engine.ff() !== "d"

        MainPage {
            id: mainPage
        }

        SettingsPage {
            id: settingsPage
        }

        AboutPage {
            id: aboutPage
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
        TabButton {
            text: qsTr("Help")
        }
    }

    Popup {
        id: whatsNewPopup

        parent: ApplicationWindow.overlay
        modal: true

        property alias text: textArea.text

        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: parent.width * 0.8; height: parent.height * 0.8

        Flickable {
            id: flickable
            anchors.fill: parent

            TextArea.flickable: TextArea {
                id: textArea
                readOnly: true
                wrapMode: TextArea.Wrap
                textFormat: TextArea.RichText
            }

            ScrollBar.vertical: ScrollBar { }
        }
    }


    Settings {
        id: news
        category: 'App'
        property string newsVersion
    }

//    Connections {
//        target: updater
//        onUpdateAvailable: {
//            whatsNewPopup.text = info
//            whatsNewPopup.open()
//        }
//    }
}
