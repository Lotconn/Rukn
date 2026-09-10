import QtQuick
// import QtCore
import QtQuick.Controls.Material
import "qrc:/qt/qml/Munadi/data/Adhan.js" as Adhan
import Munadi

QtObject {
    id: munadiModel

    property string asr: sysLocale.toString(prayerTimes.asr, isoClock ? "hh:mm" : "hh:mm ap")
    property bool bgImage: true
    property int bg_img_blur: 29
    property var calcMethod: [
        {
            "name": qsTr("Muslim World League"),
            "method": function () {
                return new Adhan.adhan.CalculationMethod.MuslimWorldLeague();
            }
        },
        {
            "name": qsTr("Egyptian"),
            "method": function () {
                return new Adhan.adhan.CalculationMethod.Egyptian();
            }
        },
        {
            "name": qsTr("Karachi"),
            "method": function () {
                return new Adhan.adhan.CalculationMethod.Karachi();
            }
        },
        {
            "name": qsTr("Umm AlQura"),
            "method": function () {
                return new Adhan.adhan.CalculationMethod.UmmAlQura();
            }
        },
        {
            "name": qsTr("Dubai"),
            "method": function () {
                return new Adhan.adhan.CalculationMethod.Dubai();
            }
        },
        {
            "name": qsTr("Moonsighting Committee"),
            "method": function () {
                return new Adhan.adhan.CalculationMethod.MoonsightingCommittee();
            }
        },
        {
            "name": qsTr("North America"),
            "method": function () {
                return new Adhan.adhan.CalculationMethod.NorthAmerica();
            }
        },
        {
            "name": qsTr("Kuwait"),
            "method": function () {
                return new Adhan.adhan.CalculationMethod.Kuwait();
            }
        },
        {
            "name": qsTr("Qatar"),
            "method": function () {
                return new Adhan.adhan.CalculationMethod.Qatar();
            }
        },
        {
            "name": qsTr("Singapore"),
            "method": function () {
                return new Adhan.adhan.CalculationMethod.Singapore();
            }
        },
        {
            "name": qsTr("Turkey"),
            "method": function () {
                return new Adhan.adhan.CalculationMethod.Turkey();
            }
        },
        {
            "name": qsTr("Other"),
            "method": function () {
                return new Adhan.adhan.CalculationMethod.Other();
            }
        }
    ]
    property int calcMethodIndex: 0
    property var calendarPreference: [
        {
            "name": qsTr("Both"),
            "method": function () {
                return gregorianDateStr + " | " + hijriDateStr;
            }
        },
        {
            "name": qsTr("Hijri"),
            "method": function () {
                return hijriDateStr;
            }
        },
        {
            "name": qsTr("Gregorian"),
            "method": function () {
                return gregorianDateStr;
            }
        }
    ]
    property int calendarPreferenceIndex: 0
    property bool checkForUpdates: true
    property string cityName
    property double content_opacity: 1.0
    property string countryName
    property date currentDate
    property string currentDateStr: calendarPreference[calendarPreferenceIndex]['method']()
    property var currentPrayer
    property date currentTime

    // To track if we're looking at different date than today
    property int dateNavOffset: 0
    property string duhr: sysLocale.toString(prayerTimes.dhuhr, isoClock ? "hh:mm" : "hh:mm ap")
    property string fajr: sysLocale.toString(prayerTimes.fajr, isoClock ? "hh:mm" : "hh:mm ap")
    property string gregorianDateStr: sysLocale.toString(currentDate, "dd-MMM-yyyy")
    property bool hideOnClose: false
    property int hijriDateOffset: 0
    property string hijriDateStr: engine.getDate(currentDate, "dd-MMM-yyyy", "IslamicCivil", hijriDateOffset)
    property int isToday: dateNavOffset === 0
    property string isha: sysLocale.toString(prayerTimes.isha, isoClock ? "hh:mm" : "hh:mm ap")
    property bool isoClock: false
    property double latitude
    property bool locationNotSet: latitude == 0 || longitude == 0
    property double longitude
    property int madhab: Adhan.adhan.Madhab.Shafi
    property string mgrb: sysLocale.toString(prayerTimes.maghrib, isoClock ? "hh:mm" : "hh:mm ap")
    property var nextPrayer
    property double overrideOffsetBy
    property double page_margins_p: 0.02
    property int pane_elevation: 6
    property var params: new Adhan.adhan.CalculationMethod.MuslimWorldLeague()
    property var prayerTimes
    property double rectangle_opacity: 0.77
    property int rectangle_radius: 3
    property bool showOnAthan: false
    property string srise: sysLocale.toString(prayerTimes.sunrise, isoClock ? "hh:mm" : "hh:mm ap")
    property bool stickerMode: false
    property var sysLocale: Qt.locale()
    property var themeList: [qsTr("System"), qsTr("Light"), qsTr("Dark")]
    property int themeListIndex: 0
    property var timeForCurrentPrayer
    property var timeForNextPrayer
    property string timeLeftStr

    signal invalidate

    function applySystemTheme() {
        mainWindow.Material.theme = Material.System;
    }

    function nextDate() {
        console.debug(new Date(), ': nextDate()');
        var nextDate = mm.currentDate;
        nextDate.setDate(nextDate.getDate() + 1);
        dateNavOffset++;
        currentDate = nextDate;
    }

    function prevDate() {
        console.debug(new Date(), ': prevDate()');
        var prevDate = mm.currentDate;
        prevDate.setDate(prevDate.getDate() - 1);
        dateNavOffset--;
        currentDate = prevDate;
    }

    function refreshTable() {
        console.debug('refreshTable()');
        currentPrayer = prayerTimes.currentPrayer();

        if (currentPrayer === Adhan.Prayer.None) {
            currentPrayer = Adhan.Prayer.Isha;
        }

        nextPrayer = prayerTimes.nextPrayer();
        timeForCurrentPrayer = prayerTimes.timeForPrayer(currentPrayer);

        if (nextPrayer === Adhan.Prayer.None && dateNavOffset === 0) {
            var tomorrow = new Date();
            tomorrow.setDate(tomorrow.getDate() + 1);
            var tomorrowPT = new Adhan.adhan.PrayerTimes(new Adhan.adhan.Coordinates(latitude, longitude), tomorrow, params);
            nextPrayer = Adhan.Prayer.Fajr;
            timeForNextPrayer = tomorrowPT.fajr;
        } else {
            timeForNextPrayer = prayerTimes.timeForPrayer(nextPrayer);
        }

        refreshTimeLeft();
    }

    function refreshTimeLeft() {
        console.debug('refreshTimeLeft()');

        if (MultimediaEngine.athanPlaying) {
            mm.timeLeftStr = qsTr("%1 Athan ...").arg(UiEngine.prayerDisplayName(mm.currentPrayer));
        } else {
            if (!mm.timeForNextPrayer) {
                mm.timeLeftStr = "";
                return;
            }

            var now = new Date();
            mm.currentTime = now;
            var totalMins = Math.ceil((mm.timeForNextPrayer.getTime() - now) / 60000);

            if (totalMins <= 0 || totalMins > 1100) { // handling extreme polar settlements
                mm.timeLeftStr = "";
                return;
            }

            var hrs = Math.floor(totalMins / 60);
            var mins = Math.floor(totalMins % 60);

            mm.timeLeftStr = '';
            if (hrs)
                mm.timeLeftStr += qsTr("%L1").arg(hrs) + "" + qsTr("h ");
            if (mins)
                mm.timeLeftStr += qsTr("%L1").arg(mins) + "" + qsTr("m ");

            if (mm.timeLeftStr) {
                mm.timeLeftStr += qsTr("until ");
            } else {
                mm.timeLeftStr += qsTr("Seconds until ");
            }

            mm.timeLeftStr += UiEngine.prayerDisplayName(mm.nextPrayer);
        }
        console.debug("timeLeftStr", mm.timeLeftStr);
    }

    function set_24h_ticker() {
        alarm.clear("24h_tick");
        var midnight = new Date();
        midnight.setHours(24, 0, 0, 0);
        alarm.add("24h_tick", midnight, 24 * 60 * 60);
    }

    function set_60s_ticker() {
        alarm.clear("60s_tick");
        var min = new Date();
        min.setSeconds(60);
        min.setMilliseconds(0);
        alarm.add("60s_tick", min, 60);
    }

    function todayDate() {
        console.debug(new Date(), ': currentDate()');
        dateNavOffset = 0;
        currentDate = new Date();
    }

    Component.onCompleted: {
        currentDate = new Date();
    }
    onCalcMethodIndexChanged: invalidate()
    onCurrentDateChanged: invalidate()
    onInvalidate: {
        console.debug(new Date(), ': invalidate()');
        var params = calcMethod[calcMethodIndex]['method']();
        params.madhab = madhab;
        params.adjustments.fajr = PrayerEngine.prayerConfig.Fajr.adjustment;
        params.adjustments.dhuhr = PrayerEngine.prayerConfig.Dhuhr.adjustment;
        params.adjustments.asr = PrayerEngine.prayerConfig.Asr.adjustment;
        params.adjustments.maghrib = PrayerEngine.prayerConfig.Maghrib.adjustment;
        params.adjustments.isha = PrayerEngine.prayerConfig.Isha.adjustment;
        mm.params = params;

        var coordinates = new Adhan.adhan.Coordinates(latitude, longitude);
        mm.prayerTimes = new Adhan.adhan.PrayerTimes(coordinates, currentDate, params);
    }
    onMadhabChanged: invalidate()
    onPrayerTimesChanged: {
        alarm.clear("athan");

        alarm.add("athan", prayerTimes.fajr);
        alarm.add("athan", prayerTimes.dhuhr);
        alarm.add("athan", prayerTimes.asr);
        alarm.add("athan", prayerTimes.maghrib);
        alarm.add("athan", prayerTimes.isha);

        refreshTable();
    }
    onThemeListIndexChanged: {
        switch (themeListIndex) {
        case 0:
            applySystemTheme();
            break;
        case 1:
            mainWindow.Material.theme = Material.Light;
            break;
        case 2:
            mainWindow.Material.theme = Material.Dark;
            break;
        }
    }
}
