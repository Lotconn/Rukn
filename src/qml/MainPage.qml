import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material

import "qrc:/qt/qml/Munadi/data/Adhan.js" as Adhan

ColumnLayout {
    anchors.fill: parent

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    property bool isWide: width > height
    property int defaultAlignment: isWide ? Layout.Left : Layout.Center

    DateRow {
        id: topBar

        Layout.preferredHeight: 40
        Layout.preferredWidth: isWide ? parent.height * theme.uiPortraitRatio : parent.width
        Layout.fillHeight: true
        Layout.alignment: parent.defaultAlignment

        background.opacity: mm.rectangle_opacity
    }

    Pane {
        id: table

        Layout.preferredHeight: 400
        Layout.preferredWidth: isWide ? parent.height * theme.uiPortraitRatio : parent.width
        Layout.fillHeight: true
        Layout.alignment: parent.defaultAlignment

        Layout.topMargin: 20
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
                Layout.fillWidth: true
            }

            PrayerRow {
                id: sunrise
                Layout.topMargin: -20
                prayerId: Adhan.adhan.Prayer.Sunrise
                athanTime: mm.srise
                Layout.preferredHeight: 30
                Layout.fillWidth: true
            }

            Line {}

            PrayerRow {
                id: duhr
                prayerId: Adhan.adhan.Prayer.Dhuhr
                athanTime: mm.duhr
                Layout.preferredHeight: 60
                Layout.fillWidth: true
            }

            Line {}

            PrayerRow {
                id: asr
                prayerId: Adhan.adhan.Prayer.Asr
                athanTime: mm.asr
                Layout.preferredHeight: 60
                Layout.fillWidth: true
            }

            Line {}

            PrayerRow {
                id: mgrb
                prayerId: Adhan.adhan.Prayer.Maghrib
                athanTime: mm.mgrb
                Layout.preferredHeight: 60
                Layout.fillWidth: true
            }

            Line {}

            PrayerRow {
                id: isha
                prayerId: Adhan.adhan.Prayer.Isha
                athanTime: mm.isha
                Layout.preferredHeight: 60
                Layout.fillWidth: true
            }
        }
    }

    TimeLeftRow {
        id: timeLeft

        Layout.preferredHeight: 40
        Layout.preferredWidth: isWide ? parent.height * theme.uiPortraitRatio : parent.width
        Layout.fillHeight: true
        Layout.alignment: defaultAlignment

        visible: !mm.locationNotSet
        text: mm.timeLeftStr

        background.opacity: mm.rectangle_opacity
    }
}
