import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1

import "qrc:/data/Adhan.js" as Adhan

// This is the main page, which showa prayer time, fav mosque selection = and time left.
Page {

    PageBorder {

        Grid {
            anchors.fill: parent
            columns: 1
            rows: mm.stickerMode ? 7 : 9

            property int cellHeight: height / rows
            property int cellWidth: width

            LocationInfoRow {
                id: locationInfoRow
                cityName: mm.cityName
                countryName: mm.countryName
            }

            DateRow {
                id: date
            }

            PrayerRow {
                id: fajr
                prayerId: Adhan.adhan.Prayer.Fajr
                prayerLabel: "Fajr"
                athanTime: mm.fajr
            }

            PrayerRow {
                id: sunrise
                prayerId: Adhan.adhan.Prayer.Sunrise
                prayerLabel: "Sunrise"
                athanTime: mm.srise
            }

            PrayerRow {
                id: duhr
                prayerId: Adhan.adhan.Prayer.Dhuhr
                prayerLabel: "Dhuhr"
                athanTime: mm.duhr
            }

            PrayerRow {
                id: asr
                prayerId: Adhan.adhan.Prayer.Asr
                prayerLabel: "Asr"
                athanTime: mm.asr
            }

            PrayerRow {
                id: mgrb
                prayerId: Adhan.adhan.Prayer.Maghrib
                prayerLabel: "Maghrib"
                athanTime: mm.mgrb
            }

            PrayerRow {
                id: isha
                prayerId: Adhan.adhan.Prayer.Isha
                prayerLabel: "Isha"
                athanTime: mm.isha
            }

            TimeLeftRow {
                id: timeLeft
                text: mm.timeLeftStr
            }
        }
    }
}
