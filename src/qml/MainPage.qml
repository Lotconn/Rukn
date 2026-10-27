import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material

import Munadi

ColumnLayout {
    id: parentBox

    property int defaultAlignment: isWide ? Layout.Left : Layout.Center
    property bool isWide: width > height

    LayoutMirroring.childrenInherit: true
    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    anchors.fill: parent

    DateRow {
        id: topBar

        Layout.alignment: parent.defaultAlignment
        Layout.fillHeight: true
        Layout.preferredHeight: 40
        Layout.preferredWidth: parentBox.isWide ? parent.height * theme.uiPortraitRatio : parent.width
        background.opacity: UiEngine.opacity
    }

    Pane {
        id: table

        property int lineHeight: height / 100

        Layout.alignment: parent.defaultAlignment
        Layout.bottomMargin: Layout.topMargin
        Layout.fillHeight: true
        Layout.preferredHeight: 400
        Layout.preferredWidth: parentBox.isWide ? parent.height * theme.uiPortraitRatio : parent.width
        Layout.topMargin: 20
        Material.elevation: mm.pane_elevation
        background.opacity: UiEngine.opacity

        ColumnLayout {
            anchors.fill: parent

            PrayerRow {
                id: fajr

                Layout.fillWidth: true
                Layout.preferredHeight: 60
                athanTime: UiEngine.prayerDisplayTimes.Fajr
                prayerId: PrayerEngine.Prayer.Fajr
            }

            PrayerRow {
                id: sunrise

                Layout.fillWidth: true
                Layout.preferredHeight: 30
                Layout.topMargin: -20
                athanTime: UiEngine.prayerDisplayTimes.Sunrise
                prayerId: PrayerEngine.Prayer.Sunrise
            }

            Line {}

            PrayerRow {
                id: duhr

                Layout.fillWidth: true
                Layout.preferredHeight: 60
                athanTime: UiEngine.prayerDisplayTimes.Dhuhr
                prayerId: PrayerEngine.Prayer.Dhuhr
            }

            Line {}

            PrayerRow {
                id: asr

                Layout.fillWidth: true
                Layout.preferredHeight: 60
                athanTime: UiEngine.prayerDisplayTimes.Asr
                prayerId: PrayerEngine.Prayer.Asr
            }

            Line {}

            PrayerRow {
                id: mgrb

                Layout.fillWidth: true
                Layout.preferredHeight: 60
                athanTime: UiEngine.prayerDisplayTimes.Maghrib
                prayerId: PrayerEngine.Prayer.Maghrib
            }

            Line {}

            PrayerRow {
                id: isha

                Layout.fillWidth: true
                Layout.preferredHeight: 60
                athanTime: UiEngine.prayerDisplayTimes.Isha
                prayerId: PrayerEngine.Prayer.Isha
            }

            Line {}

            PrayerRow {
                id: midnight

                Layout.bottomMargin: -20
                Layout.fillWidth: true
                Layout.preferredHeight: 30
                athanTime: UiEngine.prayerDisplayTimes.Midnight
                prayerId: PrayerEngine.Prayer.Isha + PrayerEngine.Sunnah.Midnight
            }

            PrayerRow {
                id: lastThirdNight

                Layout.bottomMargin: -20
                Layout.fillWidth: true
                Layout.preferredHeight: 30
                athanTime: UiEngine.prayerDisplayTimes.LastThirdNight
                prayerId: PrayerEngine.Prayer.Isha + PrayerEngine.Sunnah.LastThirdNight
            }

            Item {}
        }
    }

    TimeLeftRow {
        id: timeLeft

        Layout.alignment: parentBox.defaultAlignment
        Layout.fillHeight: true
        Layout.preferredHeight: 40
        Layout.preferredWidth: parentBox.isWide ? parent.height * theme.uiPortraitRatio : parent.width
        background.opacity: UiEngine.opacity
        text: UiEngine.timeLeftDisplay
        visible: LocationEngine.isLocationSet
    }
}
