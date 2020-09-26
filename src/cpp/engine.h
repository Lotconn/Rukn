#ifndef MUNADIENGINE_H
#define MUNADIENGINE_H

#include <QtGui>
#include <QtCore>
#include <QtDebug>

#include <version.h>

#define MUNADI_API_URL "https://munadi.org/"

extern "C" {
#include "libitl/hijri.h"
}

#include "updater.h"
#include <QStringList>

#ifdef Q_OS_ANDROID
#include <QtPositioning/QGeoCoordinate>
#include <QtPositioning/QGeoPositionInfo>
#include <QtPositioning/QGeoPositionInfoSource>
#include <QtPositioning/QGeoAddress>
#endif


class Engine : public QObject
{
    Q_OBJECT

public:

    Engine();

    Q_INVOKABLE QStringList timeZoneFromCC(const QString& cc);
    Q_INVOKABLE int findOffset(QString tzs, QDateTime dt = QDateTime::currentDateTime());

    Q_INVOKABLE QString getHijriDate(const QDate &date = QDate::currentDate(), const int offset = 0);

    Q_INVOKABLE QString getVersionNo() { return APP_VERSION; }
    Q_INVOKABLE QString getWhatsNew();
//#ifdef DESKTOP
    Q_INVOKABLE void setStartup(bool set);
//#endif

    Q_INVOKABLE QString lang()
    {
#ifdef ARABIC
    return "ar";
#else
    return "en";
#endif
    }

    Q_INVOKABLE QString ff()    // Form factor
    {
#if defined Q_OS_WIN || defined Q_OS_OSX || (defined Q_OS_LINUX && !defined Q_OS_ANDROID)
    return "d";
#else
    return "m";
#endif
    }

    //Updater * updater;
    void setStartupFlag();
    Q_INVOKABLE bool isStartupFlag();

private:

    bool startupFalg = false;

    bool init();
};

#endif // MUNADIENGINE_H
