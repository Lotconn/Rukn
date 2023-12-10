import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "qrc:/data/Adhan.js" as Adhan

ColumnLayout {
    anchors.fill: parent

    property bool isWide: width > height * 1.2
    property int defaultAlignment: isWide ? Layout.Center : Layout.Center

    DateRow {
        id: topBar

        //visible: !isWide
        buttonsVisible: !isWide

        Layout.preferredHeight: 40
        //Layout.preferredWidth: Math.min(parent.width, parent.height/1.8)
        Layout.fillHeight: true
        Layout.alignment: defaultAlignment

        Rectangle {
            anchors.fill: parent
            color: Material.backgroundColor
            opacity: mm.rectangle_opacity
            radius: mm.rectangle_radius
            z: -1
        }
    }

    Item {
        id: table

        visible: !isWide

        Layout.preferredHeight: 400
        Layout.preferredWidth: Math.min(parent.width, parent.height/1.8)
        Layout.fillHeight: true
        Layout.alignment: defaultAlignment

        Layout.topMargin: topBar.height / 3
        Layout.bottomMargin: topBar.height / 3

        property int lineHeight: height / 100

        Rectangle {
            anchors.fill: parent
            color: Material.backgroundColor
            opacity: mm.rectangle_opacity
            radius: mm.rectangle_radius
            z: -1
        }

        ColumnLayout {
            anchors.fill: parent

            PrayerRow {
                id: fajr
                prayerId: Adhan.adhan.Prayer.Fajr
                athanTime: mm.fajr
                Layout.preferredHeight: 60
                Layout.topMargin: 10
            }

            Line {}

            PrayerRow {
                id: sunrise
                prayerId: Adhan.adhan.Prayer.Sunrise
                athanTime: mm.srise
                Layout.preferredHeight: 60
            }

            Line {}

            PrayerRow {
                id: duhr
                prayerId: Adhan.adhan.Prayer.Dhuhr
                athanTime: mm.duhr
                Layout.preferredHeight: 60
            }

            Line {}

            PrayerRow {
                id: asr
                prayerId: Adhan.adhan.Prayer.Asr
                athanTime: mm.asr
                Layout.preferredHeight: 60
            }

            Line {}

            PrayerRow {
                id: mgrb
                prayerId: Adhan.adhan.Prayer.Maghrib
                athanTime: mm.mgrb
                Layout.preferredHeight: 60
            }

            Line {}

            PrayerRow {
                id: isha
                prayerId: Adhan.adhan.Prayer.Isha
                athanTime: mm.isha
                Layout.preferredHeight: 60
            }

            Line {
                Layout.bottomMargin: 10
            }
        }
    }

    TimeLeftRow {

        id: timeLeft

        visible: !mm.locationNotSet

        text: mm.timeLeftStr
        Layout.preferredHeight: 40
        //Layout.preferredWidth: Math.min(parent.width, parent.height / 1.8)
        Layout.alignment: defaultAlignment

        Rectangle {
            anchors.fill: parent
            color: Material.backgroundColor
            opacity: mm.rectangle_opacity
            radius: mm.rectangle_radius
            z: -1
        }

         RoundButton {
             visible: !isWide
             icon.source: "qrc:/img/settings.png"
             opacity: mm.content_opacity
             flat: true
             anchors.verticalCenter: parent.verticalCenter
             //anchors.horizontalCenter: parent.horizontalCenter
             anchors.left: parent.left
             //anchors.bottom: parent.bottom
             onClicked: settingsPopup.open()
         }
    }
}
