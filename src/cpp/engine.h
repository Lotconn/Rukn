#ifndef ENGINE_H
#define ENGINE_H

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

    void             start_hidden(bool);
    Q_INVOKABLE bool start_hidden();

    void             fullscreen(bool);
    Q_INVOKABLE bool fullscreen();

  private:
    bool _hidden     = false;
    bool _fullscreen = false;
};

#endif // ENGINE_H
