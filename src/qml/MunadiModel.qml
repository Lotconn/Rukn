import QtQuick 2.5
import Qt.labs.settings 1.0
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

    property date currentDate
    property date currentTime

    onCurrentDateChanged: invalidate()

    property string currentDateStr: Qt.formatDate(currentDate, Qt.DefaultLocaleLongDate)

    property bool locationNotSet: latitude == 0 || longitude == 0

    property bool stickerMode: false
    property bool showOnAthan: false
    property bool hideOnClose: false

    //    property bool checkForUpdates:  true

    property var currentPrayer
    property var nextPrayer
    property var timeForCurrentPrayer
    property var timeForNextPrayer

    function refreshTable() {
        console.debug('refreshTable()')
        currentPrayer =         prayerTimes.currentPrayer()
        nextPrayer =            prayerTimes.nextPrayer()
        timeForCurrentPrayer =  prayerTimes.timeForPrayer(currentPrayer)
        timeForNextPrayer =     prayerTimes.timeForPrayer(nextPrayer)
    }

    onTimeForNextPrayerChanged: refreshTimeLeft()

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

            var hrs = Math.floor(totalMins / 60)
            var mins = Math.floor(totalMins % 60)

            mm.timeLeftStr = ''
            if(hrs)     mm.timeLeftStr += qsTr("%L1").arg(hrs) + qsTr("h ")
            if(mins)    mm.timeLeftStr += qsTr("%L1").arg(mins) + qsTr("m ")

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
        var midnight = new Date()
        midnight.setHours(24,0,0,0)
        alarms.add("24h_tick", midnight, 24*60*60)
    }

    function set_60s_ticker() {
        var min = new Date()
        min.setSeconds(60)
        min.setMilliseconds(0)
        alarms.add("60s_tick", min, 60)
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

    property string fajr:   Qt.formatTime(prayerTimes.fajr,     "hh:mm ap")
    property string srise:  Qt.formatTime(prayerTimes.sunrise,  "hh:mm ap")
    property string duhr:   Qt.formatTime(prayerTimes.dhuhr,    "hh:mm ap")
    property string asr:    Qt.formatTime(prayerTimes.asr,      "hh:mm ap")
    property string mgrb:   Qt.formatTime(prayerTimes.maghrib,  "hh:mm ap")
    property string isha:   Qt.formatTime(prayerTimes.isha,     "hh:mm ap")

    property var prayerNames: [
        qsTr("Fajr"), qsTr("Sunrise"), qsTr("Dhuhr"), qsTr("Asr"), qsTr("Maghrib"), qsTr("Isha")]

    property var prayerConfig: [
        {"adjustment": 0, "volume": 0.2},
        {"adjustment": 0, "volume": 0.0},
        {"adjustment": 0, "volume": 1.0},
        {"adjustment": 0, "volume": 1.0},
        {"adjustment": 0, "volume": 1.0},
        {"adjustment": 0, "volume": 0.5}]

    property int audioListIndex: 0
    property var audioList: [{
            "name": qsTr("Makkah (built-in)"),
            "path": "qrc:/data/athan.ogg"
        }]

    property int themeListIndex: 0
    property var themeList: [qsTr("System"), qsTr("Light"), qsTr("Dark")]

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
