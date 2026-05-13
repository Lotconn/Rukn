import QtQuick
import QtCore
import QtQuick.Controls.Material
import "qrc:/data/Adhan.js" as Adhan

QtObject {
    id: munadiModel

    Component.onCompleted: {
        currentDate = new Date()
    }

    signal invalidate

    onInvalidate: {
        console.debug(new Date(), ': invalidate()')
        var params = calcMethod[calcMethodIndex]['method']()
        params.madhab = madhab
        params.adjustments.fajr     = prayerConfig[0].adjustment
        params.adjustments.sunrise  = prayerConfig[1].adjustment
        params.adjustments.dhuhr    = prayerConfig[2].adjustment
        params.adjustments.asr      = prayerConfig[3].adjustment
        params.adjustments.maghrib  = prayerConfig[4].adjustment
        params.adjustments.isha     = prayerConfig[5].adjustment
        mm.params = params

        var coordinates = new Adhan.adhan.Coordinates(latitude, longitude)
        mm.prayerTimes = new Adhan.adhan.PrayerTimes(coordinates, currentDate, params)
    }

    property string cityName
    property string countryName
    property double latitude
    property double longitude
    property double overrideOffsetBy

    onCurrentDateChanged: invalidate()

    property date currentDate
    property date currentTime

    // To track if we're looking at different date than today
    property int dateNavOffset: 0
    property int isToday: dateNavOffset === 0

    property int hijriDateOffset: 0
    property string calendarPreference: "Both"
    property string gregorianDateStr: Qt.formatDate(currentDate, "dd-MMM-yyyy")
    property string hijriDateStr: engine.getHijriDate(currentDate, hijriDateOffset)

    property string currentDateStr: {
        if(calendarPreference === "Both") return gregorianDateStr + " | " + hijriDateStr;
        if(calendarPreference === "Hijri") return hijriDateStr;
        if(calendarPreference === "Gregorian") return gregorianDateStr;
    }

    property bool locationNotSet: latitude == 0 || longitude == 0

    property bool stickerMode: false
    property bool showOnAthan: false
    property bool hideOnClose: false
    property bool isoClock:    false
    property bool bgImage:     true

    property int    bg_img_blur: 32
    property double rectangle_opacity: 0.8
    property double content_opacity: 1.0
    property int    rectangle_radius: 3
    property double page_margins_p: 0.02
    property int    pane_elevation: 6

    property bool checkForUpdates:  true

    property var currentPrayer
    property var nextPrayer
    property var timeForCurrentPrayer
    property var timeForNextPrayer

    function refreshTable() {
        console.debug('refreshTable()')
        currentPrayer =         prayerTimes.currentPrayer()

        if(currentPrayer === Adhan.Prayer.None) {
            currentPrayer = Adhan.Prayer.Isha
        }

        nextPrayer =            prayerTimes.nextPrayer()
        timeForCurrentPrayer =  prayerTimes.timeForPrayer(currentPrayer)

        if (nextPrayer === Adhan.Prayer.None && dateNavOffset === 0) {
            var tomorrow = new Date()
            tomorrow.setDate(tomorrow.getDate() + 1)
            var tomorrowPT = new Adhan.adhan.PrayerTimes(
                new Adhan.adhan.Coordinates(latitude, longitude),
                tomorrow,
                params)
            nextPrayer = Adhan.Prayer.Fajr
            timeForNextPrayer = tomorrowPT.fajr
        } else {
            timeForNextPrayer = prayerTimes.timeForPrayer(nextPrayer)
        }

        refreshTimeLeft()
    }

    function refreshTimeLeft() {
        console.debug('refreshTimeLeft()')

        if (athan.isOn) {
            mm.timeLeftStr = qsTr("%1 Athan ...").arg(mm.prayerNames[mm.currentPrayer])
        } else {
            if(!mm.timeForNextPrayer)
            {
                mm.timeLeftStr = ""
                return
            }

            var now = new Date()
            mm.currentTime = now
            var totalMins = Math.ceil((mm.timeForNextPrayer.getTime() - now) / 60000)

            if (totalMins <= 0) {
                mm.timeLeftStr = ""
                return
            }

            var hrs = Math.floor(totalMins / 60)
            var mins = Math.floor(totalMins % 60)

            mm.timeLeftStr = ''
            if(hrs)     mm.timeLeftStr += qsTr("%L1").arg(hrs) + "" + qsTr("h ")
            if(mins)    mm.timeLeftStr += qsTr("%L1").arg(mins) + "" + qsTr("m ")

            if(mm.timeLeftStr) {
                mm.timeLeftStr += qsTr("until ")
            } else {
                mm.timeLeftStr += qsTr("Seconds until ")
            }

            mm.timeLeftStr += mm.prayerNames[mm.nextPrayer]
        }
        console.debug("timeLeftStr", mm.timeLeftStr)
    }

    function set_24h_ticker() {
        alarms.clear("24h_tick")
        var midnight = new Date()
        midnight.setHours(24,0,0,0)
        alarms.add("24h_tick", midnight, 24*60*60)
    }

    function set_60s_ticker() {
        alarms.clear("60s_tick")
        var min = new Date()
        min.setSeconds(60)
        min.setMilliseconds(0)
        alarms.add("60s_tick", min, 60)
    }

    function todayDate() {
        console.debug(new Date(), ': currentDate()')
        dateNavOffset = 0;
        currentDate = new Date()
    }

    function prevDate() {
        console.debug(new Date(), ': prevDate()')
        var prevDate = mm.currentDate
        prevDate.setDate(prevDate.getDate() - 1)
        dateNavOffset--;
        currentDate = prevDate
    }

    function nextDate() {
        console.debug(new Date(), ': nextDate()')
        var nextDate = mm.currentDate
        nextDate.setDate(nextDate.getDate() + 1)
        dateNavOffset++;
        currentDate = nextDate
    }

    property string timeLeftStr

    property int madhab: Adhan.adhan.Madhab.Shafi
    onMadhabChanged: invalidate()

    property var prayerTimes
    onPrayerTimesChanged: {
        alarms.clear("athan")

        alarms.add("athan", prayerTimes.fajr)
        alarms.add("athan", prayerTimes.dhuhr)
        alarms.add("athan", prayerTimes.asr)
        alarms.add("athan", prayerTimes.maghrib)
        alarms.add("athan", prayerTimes.isha)

        refreshTable()
    }

    property var params: new Adhan.adhan.CalculationMethod.MuslimWorldLeague()

    property string fajr:   Qt.formatTime(prayerTimes.fajr,     isoClock   ?  "hh:mm" : "hh:mm ap")
    property string srise:  Qt.formatTime(prayerTimes.sunrise,  isoClock   ?  "hh:mm" : "hh:mm ap")
    property string duhr:   Qt.formatTime(prayerTimes.dhuhr,    isoClock   ?  "hh:mm" : "hh:mm ap")
    property string asr:    Qt.formatTime(prayerTimes.asr,      isoClock   ?  "hh:mm" : "hh:mm ap")
    property string mgrb:   Qt.formatTime(prayerTimes.maghrib,  isoClock   ?  "hh:mm" : "hh:mm ap")
    property string isha:   Qt.formatTime(prayerTimes.isha,     isoClock   ?  "hh:mm" : "hh:mm ap")

    property var prayerNames: [
        qsTr("Fajr"), qsTr("Sunrise"), qsTr("Dhuhr"), qsTr("Asr"), qsTr("Maghrib"), qsTr("Isha")]

    property var prayerConfig: [
        {"adjustment": 0, "volume": 0.2, "athan_idx": 2 },
        {"adjustment": 0, "volume": 0.0, "athan_idx": 0 },
        {"adjustment": 0, "volume": 1.0, "athan_idx": 0 },
        {"adjustment": 0, "volume": 1.0, "athan_idx": 0 },
        {"adjustment": 0, "volume": 1.0, "athan_idx": 1 },
        {"adjustment": 0, "volume": 0.5, "athan_idx": 0 }]

    property var audioList: [{
            "name": qsTr("Makkah (built-in)"),
            "path": "qrc:/data/athan-makkah.ogg"
        },
        {
            "name": qsTr("Ar-Rashid (built-in)"),
            "path": "qrc:/data/athan-arRashid.ogg"
        },
        {
            "name": qsTr("Fajr (built-in)"),
            "path": "qrc:/data/fajr-athan.ogg"
        }]

    property int themeListIndex: 0
    property var themeList: [qsTr("System"), qsTr("Light"), qsTr("Dark")]

    function applySystemTheme() {
        mainWindow.Material.theme = Material.System
    }

    onThemeListIndexChanged: {
        switch(themeListIndex) {
        case 0:
            applySystemTheme()
            break
        case 1:
            mainWindow.Material.theme = Material.Light
            break
        case 2:
            mainWindow.Material.theme = Material.Dark
            break
        }
    }

    property int calcMethodIndex: 0
    onCalcMethodIndexChanged: invalidate()

    property var calcMethod: [{
            "name": qsTr("Muslim World League"),
            "method": function () {
                return new Adhan.adhan.CalculationMethod.MuslimWorldLeague()
            }
        }, {
            "name": qsTr("Egyptian"),
            "method": function () {
                return new Adhan.adhan.CalculationMethod.Egyptian()
            }
        }, {
            "name": qsTr("Karachi"),
            "method": function () {
                return new Adhan.adhan.CalculationMethod.Karachi()
            }
        }, {
            "name": qsTr("Umm AlQura"),
            "method": function () {
                return new Adhan.adhan.CalculationMethod.UmmAlQura()
            }
        }, {
            "name": qsTr("Dubai"),
            "method": function () {
                return new Adhan.adhan.CalculationMethod.Dubai()
            }
        }, {
            "name": qsTr("Moonsighting Committee"),
            "method": function () {
                return new Adhan.adhan.CalculationMethod.MoonsightingCommittee()
            }
        }, {
            "name": qsTr("North America"),
            "method": function () {
                return new Adhan.adhan.CalculationMethod.NorthAmerica()
            }
        }, {
            "name": qsTr("Kuwait"),
            "method": function () {
                return new Adhan.adhan.CalculationMethod.Kuwait()
            }
        }, {
            "name": qsTr("Qatar"),
            "method": function () {
                return new Adhan.adhan.CalculationMethod.Qatar()
            }
        }, {
            "name": qsTr("Singapore"),
            "method": function () {
                return new Adhan.adhan.CalculationMethod.Singapore()
            }
        }, {
            "name": qsTr("Turkey"),
            "method": function () {
                return new Adhan.adhan.CalculationMethod.Turkey()
            }
        }, {
            "name": qsTr("Other"),
            "method": function () {
                return new Adhan.adhan.CalculationMethod.Other()
            }
        }]
}
