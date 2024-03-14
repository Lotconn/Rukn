#include "engine.h"
#include "libitl/hijri.h"
#include "updater.h"

#include <QTime>
#include <QSettings>
#include <QQuickView>
#include <QTextStream>
#include <QTranslator>
#include <QCalendar>

#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)

#include <QDBusInterface>
#include <QGuiApplication>

#elif defined(Q_OS_WIN)

#include <windows.h>

#endif

Engine::Engine()
{
    init();
}

QString Engine::getHijriDate(const QDate &date, const int offset)
{

    QCalendar q(QCalendar::System::Gregorian);
    return q.dateFromParts(date.year(), date.month(), date.day() +  offset).toString(QString("dd-MM-yyyy"), QCalendar(QCalendar::System::IslamicCivil));
}

void Engine::autostart_hidden(bool flag)
{
    hidden = flag;
}

bool Engine::autostart_hidden()
{
    return hidden;
}

namespace
{
#if defined(Q_OS_WIN)
    void autostart_win(bool set)
    {
        if(set)
        {
            QSettings startup("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                              QSettings::NativeFormat);
            startup.setValue(QGuiApplication::applicationName(), "\"" + QGuiApplication::applicationFilePath().replace('/','\\') + "\""+ " -startup");
        }
        else
        {
            QSettings startup("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                              QSettings::NativeFormat);
            startup.remove(QGuiApplication::applicationName());
        }
    }
#elif defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    void autostart_snap(bool set)
    {
        if(!QDir("/snap").exists())
        {
            return;
        }

        QDir autostartDir = QDir::homePath() + QDir::separator() + ".config/autostart";

        if(!autostartDir.exists())
        {
            if(!autostartDir.mkpath(autostartDir.path())) return;
        }

        QFile desktopFile("/snap/munadi/current/usr/share/applications/org.munadi.Munadi.desktop");
        QFile autostartDesktopFile(autostartDir.filePath("org.munadi.Munadi.desktop"));

        if(set)
        {
            if (!desktopFile.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                     qDebug() << "Failed to open .desktop file!";
                     return;
            }

            if (!autostartDesktopFile.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                     qDebug() << "Failed to create .desktop file!";
                     return;
            }

            QTextStream in(&desktopFile);
            QString data = in.readAll();
            data.replace("Exec=munadi", "Exec=munadi --hidden");
            QTextStream out(&autostartDesktopFile);
            out << data;
        }
        else
        {
            autostartDesktopFile.remove();
        }
    }
    void autostart_flatpak(bool set)
    {
#ifndef ANDROID
        QDBusInterface portal("org.freedesktop.portal.Desktop",
                              "/org/freedesktop/portal/desktop",
                              "org.freedesktop.portal.Background");
#endif

        QList<QString> commandline = {"munadi", "--hidden"};

        QHash<QString, QVariant> params;
        params.insert("reason", "Background Athan.");
        params.insert("autostart", set);
        params.insert("commandline", QVariant(commandline));
#ifndef ANDROID
        portal.call("RequestBackground", "", params);
#endif
    }
#elif defined(Q_OS_MAC)
    void autostart_mac(bool set)
    {
        //TODO
    }
#endif
}

void Engine::autostart(bool set)
{
#if defined(Q_OS_WIN)

    autostart_win(set);

#elif defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)

    // Can't tell if we're on Snap, Flatpak or otherwise, so do all!
    autostart_snap(set);
    autostart_flatpak(set);

#elif defined(Q_OS_MAC)

    autostart_mac(set);

#endif
}

QString Engine::getWhatsNew()
{
    QFile whats_new(":/data/whatsnew.html");
    whats_new.open(QFile::ReadOnly | QFile::Text);
    QTextStream ts(&whats_new);

    QString out = ts.readAll();

    return out;
}

bool Engine::init()
{
    //updater = new Updater();

    return true;
}
