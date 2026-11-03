import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material

import Munadi

ColumnLayout {
    property int defaultAlignment: isWide ? Layout.Left : Layout.Center
    property bool isWide: width > height
    property real preferredWidth: isWide ? height * theme.uiPortraitRatio : width

    LayoutMirroring.childrenInherit: true
    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    anchors.fill: parent

    DateRow {
        id: topBar

        Layout.alignment: parent.defaultAlignment
        Layout.fillHeight: true
        Layout.preferredHeight: 40
        Layout.preferredWidth: parent.preferredWidth
        background.opacity: UiEngine.opacity
    }

    Pane {
        id: table

        Layout.alignment: parent.defaultAlignment
        Layout.bottomMargin: Layout.topMargin
        Layout.fillHeight: true
        Layout.preferredHeight: 400
        Layout.preferredWidth: parent.preferredWidth
        Layout.topMargin: 10 * theme.unit
        Material.elevation: mm.pane_elevation
        background.opacity: UiEngine.opacity

        ColumnLayout {
            anchors.fill: parent

            PrayerRow {
                id: fajr

                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.preferredHeight: 60
                athanTime: UiEngine.prayerDisplayTimes.Fajr
                prayerId: PrayerEngine.Prayer.Fajr

                iconComponent: FajrIcon {}
            }

            PrayerRow {
                id: sunrise

                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.leftMargin: 15 * theme.unit
                Layout.preferredHeight: 30
                Layout.rightMargin: Layout.leftMargin
                Layout.topMargin: -20 * theme.unit
                athanTime: UiEngine.prayerDisplayTimes.Sunrise
                muted: true
                prayerId: PrayerEngine.Prayer.Sunrise
            }

            Line {}

            PrayerRow {
                id: duhr

                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.preferredHeight: 60
                athanTime: UiEngine.prayerDisplayTimes.Dhuhr
                prayerId: PrayerEngine.Prayer.Dhuhr

                iconComponent: DhuhrIcon {}
            }

            Line {}

            PrayerRow {
                id: asr

                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.preferredHeight: 60
                athanTime: UiEngine.prayerDisplayTimes.Asr
                prayerId: PrayerEngine.Prayer.Asr

                iconComponent: AsrIcon {}
            }

            Line {}

            PrayerRow {
                id: mgrb

                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.preferredHeight: 60
                athanTime: UiEngine.prayerDisplayTimes.Maghrib
                prayerId: PrayerEngine.Prayer.Maghrib

                iconComponent: MaghribIcon {}
            }

            Line {}

            PrayerRow {
                id: isha

                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.preferredHeight: 60
                athanTime: UiEngine.prayerDisplayTimes.Isha
                prayerId: PrayerEngine.Prayer.Isha

                iconComponent: IshaIcon {}
            }

            PrayerRow {
                id: midnight

                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.leftMargin: 15 * theme.unit
                Layout.preferredHeight: 30
                Layout.rightMargin: Layout.leftMargin
                Layout.topMargin: -20 * theme.unit
                athanTime: UiEngine.prayerDisplayTimes.Midnight
                muted: true
                prayerId: PrayerEngine.Prayer.Isha + PrayerEngine.Sunnah.Midnight
            }

            PrayerRow {
                id: lastThirdNight

                Layout.bottomMargin: -5 * theme.unit
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.leftMargin: 15 * theme.unit
                Layout.preferredHeight: 30
                Layout.rightMargin: Layout.leftMargin
                Layout.topMargin: -15 * theme.unit
                athanTime: UiEngine.prayerDisplayTimes.LastThirdNight
                muted: true
                prayerId: PrayerEngine.Prayer.Isha + PrayerEngine.Sunnah.LastThirdNight
            }

            Line {}
        }
    }

    TimeLeftRow {
        id: timeLeft

        Layout.alignment: parent.defaultAlignment
        Layout.fillHeight: true
        Layout.preferredHeight: 40
        Layout.preferredWidth: parent.preferredWidth
        background.opacity: UiEngine.opacity
        text: UiEngine.timeLeftDisplay
        visible: LocationEngine.isLocationSet
    }
}
