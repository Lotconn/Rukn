#ifndef MUNADIENGINE_H
#define MUNADIENGINE_H

#include <QObject>

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
    explicit Engine() = default;

    Q_INVOKABLE QString getWhatsNew();
    QString             show_help(bool);

    Q_INVOKABLE void autostart(bool set);

    void             autostart_hidden(bool);
    Q_INVOKABLE bool autostart_hidden();

    void             fullscreen(bool);
    Q_INVOKABLE bool fullscreen();

  private:
    bool _hidden     = false;
    bool _fullscreen = false;
};

#endif // MUNADIENGINE_H
