import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "qrc:/data/Adhan.js" as Adhan

ColumnLayout {
    anchors.fill: parent

    property bool isWide: width > height * 1.2
    property int defaultAlignment: isWide ? Layout.Left : Layout.Center

    DateRow {
        id: topBar

        Layout.preferredHeight: 40
        Layout.preferredWidth: Math.min(parent.width, parent.height/1.8)
        Layout.fillHeight: true
        Layout.alignment: defaultAlignment

        background.opacity: mm.rectangle_opacity
    }

    Pane {
        id: table

        Layout.preferredHeight: 400
        Layout.preferredWidth: Math.min(parent.width, parent.height/1.8)
        Layout.fillHeight: true
        Layout.alignment: defaultAlignment

        Layout.topMargin: topBar.height / 4
        Layout.bottomMargin: Layout.topMargin

        Material.elevation: mm.pane_elevation

        background.opacity: mm.rectangle_opacity

        property int lineHeight: height / 100

        ColumnLayout {
            anchors.fill: parent

            PrayerRow {
                id: fajr
                prayerId: Adhan.adhan.Prayer.Fajr
                athanTime: mm.fajr
                Layout.preferredHeight: 60
            }

            PrayerRow {
                id: sunrise
                Layout.topMargin: -20
                prayerId: Adhan.adhan.Prayer.Sunrise
                athanTime: mm.srise
                Layout.preferredHeight: 30
                //color: "red"
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
        }
    }

    TimeLeftRow {
        id: timeLeft

        Layout.preferredHeight: 40
        Layout.preferredWidth: Math.min(parent.width, parent.height / 1.8)
        Layout.alignment: defaultAlignment

        visible: !mm.locationNotSet
        text: mm.timeLeftStr

        background.opacity: mm.rectangle_opacity

         Button {
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
