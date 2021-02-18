import QtQuick 2.5
import Qt.labs.settings 1.0
import "qrc:/data/Adhan.js" as Adhan

QtObject {
    id: munadiModel

    Component.onCompleted: {
        currentDate = new Date()
        invalidate()
    }

    signal invalidate

    onInvalidate: {
        var params = calcMethod[calcMethodIndex]['method']()
        params.madhab = madhab
        params.adjustments.fajr     = prayerConfig[0].adjustment
        params.adjustments.sunrise  = prayerConfig[1].adjustment
        params.adjustments.dhuhr    = prayerConfig[2].adjustment
        params.adjustments.asr      = prayerConfig[3].adjustment
        params.adjustments.maghrib  = prayerConfig[4].adjustment
        params.adjustments.isha     = prayerConfig[5].adjustment
        mm.params = params
        currentDate = new Date()
    }

    property date currentDate
    property string currentDateStr: /*calendarPreference === "Hijri" ? hijriDateStr :*/ gregorianDateStr
    property string gregorianDateStr: Qt.formatDate(currentDate, Qt.locale(), Locale.LongFormat) //TODO make date format configurable
    property string hijriDateStr: engine.getHijriDate(currentDate,
                                                      hijriDateOffset)
    property int hijriDateOffset
    property string calendarPreference

    property string cityName
    property string countryName
    property double latitude
    property double longitude
    property double overrideOffsetBy

    property bool locationNotSet: latitude == 0 || longitude == 0

    property bool stickerMode: false
    property bool showOnAthan: false

    //    property bool checkForUpdates:  true
    property var currentPrayer:         prayerTimes.currentPrayer(currentDate)
    property var nextPrayer:            prayerTimes.nextPrayer(currentDate)
    property var timeForCurrentPrayer:  prayerTimes.timeForPrayer(currentPrayer)
    property var timeForNextPrayer:     prayerTimes.timeForPrayer(nextPrayer)

    property string timeLeftStr

    property var coordinates: new Adhan.adhan.Coordinates(latitude, longitude)
    property var prayerTimes: new Adhan.adhan.PrayerTimes(coordinates,
                                                          currentDate, params)
    property var formattedTime: Adhan.adhan.Date.formattedTime

    property int madhab: Adhan.adhan.Madhab.Shafi
    property var params: new Adhan.adhan.CalculationMethod.MuslimWorldLeague()

    onMadhabChanged: invalidate()

    property string fajr:   Qt.formatTime(prayerTimes.fajr, "hh:mm ap")
    property string srise:  Qt.formatTime(prayerTimes.sunrise, "hh:mm ap")
    property string duhr:   Qt.formatTime(prayerTimes.dhuhr, "hh:mm ap")
    property string asr:    Qt.formatTime(prayerTimes.asr, "hh:mm ap")
    property string mgrb:   Qt.formatTime(prayerTimes.maghrib, "hh:mm ap")
    property string isha:   Qt.formatTime(prayerTimes.isha, "hh:mm ap")

    property var prayerNames: [qsTr("Fajr"), qsTr("Sunrise"), qsTr(
            "Dhuhr"), qsTr("Asr"), qsTr("Maghrib"), qsTr("Isha")]

    property var prayerConfig: [{
            "adjustment": 0,
            "volume": 0.2
        }, {
            "adjustment": 0,
            "volume": 0.0
        }, {
            "adjustment": 0,
            "volume": 1.0
        }, {
            "adjustment": 0,
            "volume": 1.0
        }, {
            "adjustment": 0,
            "volume": 1.0
        }, {
            "adjustment": 0,
            "volume": 0.5
        }]

    property int audioListIndex: 0
    property var audioList: [{
            "name": qsTr("Makkah (built-in)"),
            "path": "qrc:/data/athan.ogg"
        }]

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
