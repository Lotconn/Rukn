import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.1

import "qrc:/data/Adhan.js" as Adhan

// This is the main page, which showa prayer time, fav mosque selection = and time left.
Page {
    ColumnLayout {
        anchors.fill: parent

        LocationInfoRow {
            id: locationInfoRow
            cityName: mm.cityName
            countryName: mm.countryName
            Layout.preferredHeight: 20
        }

        DateRow {
            id: date
            Layout.preferredHeight: 60
        }

        PrayerRow {
            id: fajr
            prayerId: Adhan.adhan.Prayer.Fajr
            athanTime: mm.fajr
            Layout.preferredHeight: 60
        }

        PrayerRow {
            id: sunrise
            prayerId: Adhan.adhan.Prayer.Sunrise
            athanTime: mm.srise
            Layout.preferredHeight: 30
        }

        Line {
            Layout.preferredWidth: parent.width - parent.width / 8
            Layout.alignment: Qt.AlignHCenter
        }

        PrayerRow {
            id: duhr
            prayerId: Adhan.adhan.Prayer.Dhuhr
            athanTime: mm.duhr
            Layout.preferredHeight: 60
        }

        Line {
            Layout.preferredWidth: parent.width - parent.width / 8
            Layout.alignment: Qt.AlignHCenter
        }

        PrayerRow {
            id: asr
            prayerId: Adhan.adhan.Prayer.Asr
            athanTime: mm.asr
            Layout.preferredHeight: 60
        }

        Line {
            Layout.preferredWidth: parent.width - parent.width / 8
            Layout.alignment: Qt.AlignHCenter
        }

        PrayerRow {
            id: mgrb
            prayerId: Adhan.adhan.Prayer.Maghrib
            athanTime: mm.mgrb
            Layout.preferredHeight: 60
        }

        Line {
            Layout.preferredWidth: parent.width - parent.width / 8
            Layout.alignment: Qt.AlignHCenter
        }

        PrayerRow {
            id: isha
            prayerId: Adhan.adhan.Prayer.Isha
            athanTime: mm.isha
            Layout.preferredHeight: 60
        }

        Line {
            Layout.preferredWidth: parent.width - parent.width / 8
            Layout.alignment: Qt.AlignHCenter
        }

        TimeLeftRow {
            id: timeLeft
            text: mm.timeLeftStr
             Layout.preferredHeight: 40

             RoundButton {
                 text: "⚙"
                 opacity: 0.5
                 flat: true
                 anchors.verticalCenter: parent.verticalCenter
                 anchors.left: parent.left
                 anchors.bottom: parent.bottom
                 onClicked: settingsPopup.open()
             }
        }
    }
}
