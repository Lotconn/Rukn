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

QStringList Engine::timeZoneFromCC(const QString &cc)
{
        QString CC = cc.toUpper();

        if(CC == "") return QStringList();

        QFile f(":/data/timezones.tsv");

        if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        {
                 qDebug() << "Failed to open file!";
                 return QStringList();
        }

        QTextStream ts(&f);

        QStringList timezones;
        QString line;

        while(!ts.atEnd())
        {
            line = ts.readLine();

            if(line.startsWith(CC)) {
                timezones << line.split("\t").at(1);
            }
        }

        return timezones;
}

int Engine::findOffset(QString tzs, QDateTime dt)
{
    QTimeZone tz(tzs.toLatin1());
    int offset = tz.standardTimeOffset(dt) / 3600;
    int dst = tz.isDaylightTime(dt);

    return dst ? offset + 1 : offset;
}
void Engine::showMainWindow()
{
//    parent->setWindowState( Qt::WindowState(parent->windowState() & ~Qt::WindowMinimized) );
//    parent->show();
//    parent->showNormal();
//    parent->raise();  // for MacOS
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

    hijriDate = hijriDate.append(hDate.to_mname);
    hijriDate = hijriDate.append(" ");
    hijriDate = hijriDate.append(QString::number(hDate.day));
    hijriDate = hijriDate.append(" ");
    hijriDate = hijriDate.append(QString::number(hDate.year));

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

//    if(!autostartDir.exists())
//    {
//        if(!autostartDir.mkpath(autostartDir.path())) return;
//    }

    QFile desktopFile("/app/share/applications/org.munadi.Munadi.desktop");
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
        data.replace("Exec=munadi", "Exec=flatpak run org.munadi.Munadi --hidden");
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
