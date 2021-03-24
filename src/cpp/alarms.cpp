#include <unistd.h>
#include <signal.h>

#include "engine.h"

#include "alarms.h"

Alarms::Alarms(QObject *parent) : QObject(parent)
{
    signal(SIGALRM, Alarms::sighandler);
    emit empty();
}
Alarms& Alarms::instance()
{
    static Alarms alarms;
    return alarms;
}
void Alarms::sighandler(int i)
{
    if(i == SIGALRM)
    {
        Alarms& alarms = Alarms::instance();
        emit alarms.trigger();
        alarms.prepNextAlarm();
        alarm(alarms.secsToNextAlarm());
        qDebug() << "Alarm triggered !!! " << QDateTime::currentDateTimeUtc();
    }
}
void Alarms::add(QDateTime&& date_time)
{
    qDebug() << "New alarm for: " << date_time;
    entries.insert(date_time);

    alarm(secsToNextAlarm());
}
void Alarms::clear()
{
    entries.clear();
    alarm(0);
}
void Alarms::prepNextAlarm()
{
    if(entries.empty())
    {
        emit empty();
        return;
    }

    auto it = entries.begin();
    entries.erase(it);
}
qint64 Alarms::secsToNextAlarm()
{
    if(entries.empty()) return 0;

    auto it = entries.begin();
    auto dt = *it;
    auto diff = dt.toSecsSinceEpoch() - QDateTime::currentSecsSinceEpoch();

    return diff;
}
