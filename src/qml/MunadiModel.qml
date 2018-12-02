import QtQuick 2.5
import Qt.labs.settings 1.0
import "qrc:/data/Adhan.js" as Adhan

QtObject {
    id: munadiModel

    Component.onCompleted: {
        currentDate = new Date();
    }

    signal invalidate()

    function mapCalcMethod(method) {
        switch(method) {
            case qsTr("Muslim World League"):       return new Adhan.adhan.CalculationMethod.MuslimWorldLeague();
            case qsTr("Egyptian"):                  return new Adhan.adhan.CalculationMethod.Egyptian();
            case qsTr("Karachi"):                   return new Adhan.adhan.CalculationMethod.Karachi();
            case qsTr("Umm AlQura"):                return new Adhan.adhan.CalculationMethod.UmmAlQura();
            case qsTr("Gulf"):                      return new Adhan.adhan.CalculationMethod.Gulf();
            case qsTr("Qatar"):                     return new Adhan.adhan.CalculationMethod.Qatar();
            case qsTr("Kuwait"):                    return new Adhan.adhan.CalculationMethod.Kuwait();
            case qsTr("Moonsighting Committee"):    return new Adhan.adhan.CalculationMethod.MoonsightingCommittee();
            case qsTr("North America"):             return new Adhan.adhan.CalculationMethod.NorthAmerica();
            case qsTr("Other"):                     return new Adhan.adhan.CalculationMethod.Other();
        }
    }

    function mapPrayerName(prayer) {
        switch(prayer) {
            case Adhan.adhan.Prayer.Fajr:      return qsTr("Fajr");
            case Adhan.adhan.Prayer.Sunrise:   return qsTr("Srise");
            case Adhan.adhan.Prayer.Dhuhr:     return qsTr("Duhr");
            case Adhan.adhan.Prayer.Asr:       return qsTr("Asr");
            case Adhan.adhan.Prayer.Maghrib:   return qsTr("Mgrb");
            case Adhan.adhan.Prayer.Isha:      return qsTr("Isha");
        }
    }

    onInvalidate: {
        var params = mapCalcMethod(calcMethod)
        params.madhab               = madhab
        params.adjustments.fajr     = fajrAdjustment
        params.adjustments.sunrise  = sriseAdjustment
        params.adjustments.dhuhr    = duhrAdjustment
        params.adjustments.asr      = asrAdjustment
        params.adjustments.maghrib  = mgrbAdjustment
        params.adjustments.isha     = ishaAdjustment
        mm.params = params
        currentDate = new Date()
    }


    property date currentDate
    property string currentDateStr:     calendarPreference === qsTr("Hijri") ? hijriDateStr : gregorianDateStr
    property string gregorianDateStr:   Qt.formatDate(currentDate, Qt.TextDate) //TODO make date format configurable
    property string hijriDateStr:       engine.getHijriDate(currentDate, hijriDateOffset)
    property int hijriDateOffset;       //onHijriDateOffsetChanged: {hijriDateStr = engine.getHijriDate(currentDate, hijriDateOffset); calendarPreference = qsTr("Hijri")}
    property string calendarPreference
    property string dateFormat: "24h"

    property string cityName: ""
    property string stateName
    property string countryName
    property string timeZone
    property double latitude
    property double longitude
    property double overrideOffsetBy
    property double utcOffset: engine.findOffset(timeZone, currentDate) + overrideOffsetBy

    property bool locationNotSet: latitude == 0 || longitude == 0

    property bool stickerMode:      false
    property bool showOnAthan:      false
//    property bool checkForUpdates:  true

    property var nextPrayer:            prayerTimes.nextPrayer(currentDate)
    property var currentPrayer:         prayerTimes.currentPrayer(currentDate)
    property var timeForCurrentPrayer:  prayerTimes.timeForPrayer(currentPrayer)
    property var timeForNextPrayer:     prayerTimes.timeForPrayer(nextPrayer)

    property string timeLeftStr


    property var coordinates:   new Adhan.adhan.Coordinates(latitude, longitude)
    property var prayerTimes:   new Adhan.adhan.PrayerTimes(coordinates, currentDate, params)
    property var formattedTime: Adhan.adhan.Date.formattedTime

    property var calcMethod:        qsTr("Muslim World League")
    property int madhab:            Adhan.adhan.Madhab.Shafi
    property var params:            new Adhan.adhan.CalculationMethod.MuslimWorldLeague()


    onCalcMethodChanged:    invalidate()
    onMadhabChanged:        invalidate()

    property string fajr:   formattedTime(prayerTimes.fajr, utcOffset, dateFormat)
    property string srise:  formattedTime(prayerTimes.sunrise, utcOffset, dateFormat)
    property string duhr:   formattedTime(prayerTimes.dhuhr, utcOffset, dateFormat)
    property string asr:    formattedTime(prayerTimes.asr, utcOffset, dateFormat)
    property string mgrb:   formattedTime(prayerTimes.maghrib, utcOffset, dateFormat)
    property string isha:   formattedTime(prayerTimes.isha, utcOffset, dateFormat)


    property int fajrAdjustment
    property int sriseAdjustment
    property int duhrAdjustment
    property int asrAdjustment
    property int mgrbAdjustment
    property int ishaAdjustment
}
