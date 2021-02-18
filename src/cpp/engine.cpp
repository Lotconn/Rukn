#include "engine.h"
#include "libitl/hijri.h"
#include "updater.h"

#include <QTime>
#include <QSettings>
#include <QQuickView>
#include <QTextStream>

#ifdef DESKTOP

#include <QGuiApplication>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#endif

Engine::Engine()
{
    init();
}

QString Engine::getHijriDate(const QDate &date, const int offset)
{
    sDate hDate;

    QDate adjDate = date.addDays(offset);

    // Convert using hijri code from meladi to hijri
    int ret = h_date(&hDate, adjDate.day(), adjDate.month(), adjDate.year());

    if (ret)
        return "";

    QString hijriDate;

    hijriDate = hijriDate.append(QString::number(hDate.year));
    hijriDate = hijriDate.append(".");
    hijriDate = hijriDate.append(QString::number(hDate.month));
    hijriDate = hijriDate.append(".");
    hijriDate = hijriDate.append(QString::number(hDate.day));

    return hijriDate;
}

void Engine::setStartupFlag()
{
    startupFalg = true;
}

bool Engine::isStartupFlag()
{
    return startupFalg;
}

void Engine::setStartup(bool set)
{
#if defined(Q_OS_WIN)

    //qDebug() << "void Engine::setStarup(bool set)";

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
#elif defined(Q_OS_LINUX)
    QDir autostartDir = QDir::homePath() + QDir::separator() + ".config/autostart";

    if(!autostartDir.exists())
    {
        if(!autostartDir.mkpath(autostartDir.path())) return;
    }
    
    // Assume Snap for now, Flatpak has no support for autostart as of 2019.12
    QFile desktopFile("/snap/munadi/current/usr/share/applications/org.munadi.Munadi.desktop");
    QFile autostartDesktopFile(autostartDir.filePath("org.munadi.Munadi.desktop"));

    if(set)
    {
        if (!desktopFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
                 qDebug() << "Failed to open file!";
                 return;
        }

        if (!autostartDesktopFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
                 qDebug() << "Failed to create file!";
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
