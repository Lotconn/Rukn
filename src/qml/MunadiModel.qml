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

    function mapCalcMethod(method) {
        switch (method) {
        case qsTr("Muslim World League"):
            return new Adhan.adhan.CalculationMethod.MuslimWorldLeague()
        case qsTr("Egyptian"):
            return new Adhan.adhan.CalculationMethod.Egyptian()
        case qsTr("Karachi"):
            return new Adhan.adhan.CalculationMethod.Karachi()
        case qsTr("Umm AlQura"):
            return new Adhan.adhan.CalculationMethod.UmmAlQura()
        case qsTr("Dubai"):
            return new Adhan.adhan.CalculationMethod.Dubai()
        case qsTr("Moonsighting Committee"):
            return new Adhan.adhan.CalculationMethod.MoonsightingCommittee()
        case qsTr("North America"):
            return new Adhan.adhan.CalculationMethod.NorthAmerica()
        case qsTr("Kuwait"):
            return new Adhan.adhan.CalculationMethod.Kuwait()
        case qsTr("Qatar"):
            return new Adhan.adhan.CalculationMethod.Qatar()
        case qsTr("Singapore"):
            return new Adhan.adhan.CalculationMethod.Singapore()
        case qsTr("Turkey"):
            return new Adhan.adhan.CalculationMethod.Turkey()
        case qsTr("Other"):
            return new Adhan.adhan.CalculationMethod.Other()
        }
    }

    property var prayerNames: [qsTr("Fajr"), qsTr("Sunrise"), qsTr(
            "Dhuhr"), qsTr("Asr"), qsTr("Maghrib"), qsTr("Isha")]

    onInvalidate: {
        var params = mapCalcMethod(calcMethod)
        params.madhab = madhab
        params.adjustments.fajr = prayerConfig[0].adjustment
        params.adjustments.sunrise = prayerConfig[1].adjustment
        params.adjustments.dhuhr = prayerConfig[2].adjustment
        params.adjustments.asr = prayerConfig[3].adjustment
        params.adjustments.maghrib = prayerConfig[4].adjustment
        params.adjustments.isha = prayerConfig[5].adjustment
        mm.params = params
        currentDate = new Date()
    }

    property date currentDate
    property string currentDateStr: calendarPreference === qsTr(
                                        "Hijri") ? hijriDateStr : gregorianDateStr
    property string gregorianDateStr: Qt.formatDate(
                                          currentDate,
                                          Qt.TextDate) //TODO make date format configurable
    property string hijriDateStr: engine.getHijriDate(currentDate,
                                                      hijriDateOffset)
    property int hijriDateOffset
    //onHijriDateOffsetChanged: {hijriDateStr = engine.getHijriDate(currentDate, hijriDateOffset); calendarPreference = qsTr("Hijri")}
    property string calendarPreference
    property string dateFormat: "24h"

    property string cityName
    property string countryName
    property string timeZone
    property double latitude
    property double longitude
    property double overrideOffsetBy
    property double utcOffset: engine.findOffset(timeZone,
                                                 currentDate) + overrideOffsetBy

    property bool locationNotSet: latitude == 0 || longitude == 0

    property bool stickerMode: false
    property bool showOnAthan: false

    //    property bool checkForUpdates:  true
    property var currentPrayer: prayerTimes.currentPrayer(currentDate)
    property var nextPrayer: prayerTimes.nextPrayer(currentDate)
    property var timeForCurrentPrayer: prayerTimes.timeForPrayer(currentPrayer)
    property var timeForNextPrayer: prayerTimes.timeForPrayer(nextPrayer)

    property string timeLeftStr

    property var coordinates: new Adhan.adhan.Coordinates(latitude, longitude)
    property var prayerTimes: new Adhan.adhan.PrayerTimes(coordinates,
                                                          currentDate, params)
    property var formattedTime: Adhan.adhan.Date.formattedTime

    property var calcMethod: qsTr("Muslim World League")
    property int madhab: Adhan.adhan.Madhab.Shafi
    property var params: new Adhan.adhan.CalculationMethod.MuslimWorldLeague()

    onCalcMethodChanged: invalidate()
    onMadhabChanged: invalidate()

    property string fajr: formattedTime(prayerTimes.fajr, utcOffset, dateFormat)
    property string srise: formattedTime(prayerTimes.sunrise, utcOffset,
                                         dateFormat)
    property string duhr: formattedTime(prayerTimes.dhuhr, utcOffset,
                                        dateFormat)
    property string asr: formattedTime(prayerTimes.asr, utcOffset, dateFormat)
    property string mgrb: formattedTime(prayerTimes.maghrib, utcOffset,
                                        dateFormat)
    property string isha: formattedTime(prayerTimes.isha, utcOffset, dateFormat)

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
}
