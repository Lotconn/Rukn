#include <engine.h>
#include <updater.h>

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
    const QDate &d = date.addDays(offset);
    QCalendar q(QCalendar::System::Gregorian);
    return q.dateFromParts(d.year(), d.month(), d.day()).toString(QString("dd-MMM-yyyy"), QCalendar(QCalendar::System::IslamicCivil));
}

QString Engine::show_help(bool flag)
{
    return "\nUsage: munadi [options]\n"
    "Options:\n"
    "  --hidden        Start minimized to system tray\n"
    "  --fullscreen    Start in fullscreen mode\n"
    "  --help          Show this help message and exit\n\n";
}

void Engine::autostart_hidden(bool flag)
{
    _hidden = flag;
}

bool Engine::autostart_hidden()
{
    return _hidden;
}

void Engine::fullscreen(bool flag)
{
    _fullscreen = flag;
}

bool Engine::fullscreen()
{
    return _fullscreen;
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
    void autostart_linux(bool set)
    {
        QDir autostartDir = QDir::homePath() + QDir::separator() + ".config/autostart";

        if(!autostartDir.exists())
        {
            if(!autostartDir.mkpath(autostartDir.path())) return;
        }

        QString desktopPath;
        QString localPath = QDir::homePath() + "/.local/share/applications/org.munadi.Munadi.desktop";
        QString systemPath = "/usr/local/share/applications/org.munadi.Munadi.desktop";

        if (QFile::exists(localPath)) {
            desktopPath = localPath;
        } else {
            desktopPath = systemPath;
        }
        QFile desktopFile(desktopPath);

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

            QString data = QTextStream(&desktopFile).readAll();
            data.replace("bin/munadi", "bin/munadi --hidden");
            QTextStream(&autostartDesktopFile) << data;
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

        QList<QString> commandline = {"munadi", "--hidden"};

        QHash<QString, QVariant> params;
        params.insert("reason", "Background Athan.");
        params.insert("autostart", set);
        params.insert("commandline", QVariant(commandline));
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

    if(qEnvironmentVariableIsSet("FLATPAK_ID"))
        autostart_flatpak(set);
    else
        autostart_linux(set);

#elif defined(Q_OS_MAC)

    autostart_mac(set);

#endif
}

QString Engine::getWhatsNew()
{
    QFile whats_new(":/qt/qml/Munadi/data/whatsnew.html");
    std::ignore = whats_new.open(QFile::ReadOnly | QFile::Text);
    QTextStream ts(&whats_new);

    QString out = ts.readAll();

    return out;
}

bool Engine::init()
{
    updater = new Updater();

    return true;
}
