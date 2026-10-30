#ifndef MUNADIENGINE_H
#define MUNADIENGINE_H

#include <QtCore>
#include <QtDebug>
#include <QtGui>

#define MUNADI_API_URL "https://munadi.org/"

#include <UpdateEngine.hpp>

#ifdef Q_OS_ANDROID
#include <QtPositioning/QGeoAddress>
#include <QtPositioning/QGeoCoordinate>
#include <QtPositioning/QGeoPositionInfo>
#include <QtPositioning/QGeoPositionInfoSource>
#endif

class Engine : public QObject
{
    Q_OBJECT

  public:
    Engine();

    Q_INVOKABLE QString getWhatsNew();
    QString             show_help(bool);

    Q_INVOKABLE void autostart(bool set);

    Updater         *updater;
    void             autostart_hidden(bool);
    Q_INVOKABLE bool autostart_hidden();

    void             fullscreen(bool);
    Q_INVOKABLE bool fullscreen();

  private:
    bool _hidden     = false;
    bool _fullscreen = false;

    bool init();
};

#endif // MUNADIENGINE_H
