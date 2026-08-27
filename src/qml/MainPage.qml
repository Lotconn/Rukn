import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material

import "qrc:/qt/qml/Munadi/data/Adhan.js" as Adhan
import Munadi

ColumnLayout {
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
        Layout.preferredWidth: isWide ? parent.height * theme.uiPortraitRatio : parent.width
        background.opacity: mm.rectangle_opacity
    }

    Pane {
        id: table

        property int lineHeight: height / 100

        Layout.alignment: parent.defaultAlignment
        Layout.bottomMargin: Layout.topMargin
        Layout.fillHeight: true
        Layout.preferredHeight: 400
        Layout.preferredWidth: isWide ? parent.height * theme.uiPortraitRatio : parent.width
        Layout.topMargin: 20
        Material.elevation: mm.pane_elevation
        background.opacity: mm.rectangle_opacity

        ColumnLayout {
            anchors.fill: parent

            PrayerRow {
                id: fajr

                Layout.fillWidth: true
                Layout.preferredHeight: 60
                athanTime: mm.fajr
                prayerId: PrayerEngine.Fajr
            }

            PrayerRow {
                id: sunrise

                Layout.fillWidth: true
                Layout.preferredHeight: 30
                Layout.topMargin: -20
                athanTime: mm.srise
                prayerId: PrayerEngine.Sunrise
            }

            Line {}

            PrayerRow {
                id: duhr

                Layout.fillWidth: true
                Layout.preferredHeight: 60
                athanTime: mm.duhr
                prayerId: PrayerEngine.Dhuhr
            }

            Line {}

            PrayerRow {
                id: asr

                Layout.fillWidth: true
                Layout.preferredHeight: 60
                athanTime: mm.asr
                prayerId: PrayerEngine.Asr
            }

            Line {}

            PrayerRow {
                id: mgrb

                Layout.fillWidth: true
                Layout.preferredHeight: 60
                athanTime: mm.mgrb
                prayerId: PrayerEngine.Maghrib
            }

            Line {}

            PrayerRow {
                id: isha

                Layout.fillWidth: true
                Layout.preferredHeight: 60
                athanTime: mm.isha
                prayerId: PrayerEngine.Isha
            }
        }
    }

    TimeLeftRow {
        id: timeLeft

        Layout.alignment: defaultAlignment
        Layout.fillHeight: true
        Layout.preferredHeight: 40
        Layout.preferredWidth: isWide ? parent.height * theme.uiPortraitRatio : parent.width
        background.opacity: mm.rectangle_opacity
        text: mm.timeLeftStr
        visible: !mm.locationNotSet
    }
}
