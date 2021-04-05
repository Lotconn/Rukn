#include <unistd.h>
#include <signal.h>

#include "engine.h"

#include "alarms.h"

Alarms::Alarms(QObject *parent) : QObject(parent)
{
    signal(SIGALRM, Alarms::sighandler);
}
Alarms& Alarms::instance()
{
    static Alarms alarms;
    return alarms;
}
void Alarms::sighandler(int i)
{
    if(i != SIGALRM) return;

    static Alarms& alarms = Alarms::instance();

    alarms.prepNextAlarm();

    alarm(alarms.secsToNextAlarm());
}
void Alarms::add(QDateTime dt, QString id, qint64 interval)
{
    if(auto epoch = dt.toSecsSinceEpoch(); epoch > QDateTime::currentSecsSinceEpoch())
    {
        entries[epoch].emplace_back(Meta{id, interval});
        alarm(secsToNextAlarm());
        qDebug() << "New alarm for: " << QDateTime::fromSecsSinceEpoch(epoch) << ", id: "
                    << id << ", interval: " << interval << ", count: " << entries.size();
    }
}
void Alarms::clear(QString id)
{
    for(auto it = std::begin(entries); it != std::end(entries);)
    {
        auto& meta_list = it->second;
        meta_list.erase(std::remove_if(meta_list.begin(), meta_list.end(),
                                      [&](Meta& meta){return meta.id == id;}) ,meta_list.end());

        if(meta_list.size())
        {
            ++it;
        }
        else
        {
            it = entries.erase(it);
        }
    }

    qDebug() << "Alarm count: " << entries.size();

    alarm(secsToNextAlarm());
}
void Alarms::prepNextAlarm()
{
    if(entries.empty())
    {
        emit empty();
        return;
    }

    auto it = entries.begin();
    auto epoch = it->first;
    auto meta_list = it->second;
    entries.erase(it);

    for(auto& meta : meta_list)
    {
        emit trigger(meta.id);
        qDebug() << "Alarm triggered !!! " << QDateTime::currentDateTime() << ", id: " << meta.id;

        if(meta.interval)
        {
            add(QDateTime::fromSecsSinceEpoch(epoch + meta.interval), meta.id, meta.interval);
        }
    }
}
qint64 Alarms::secsToNextAlarm()
{
    if(entries.empty()) return 0;

    auto it = entries.begin();
    auto epoch = it->first;
    auto diff = epoch - QDateTime::currentSecsSinceEpoch();

    if(diff > 0) return diff;

    // We shouldn't reach here, else probably delayed signal, let's catch up!
    prepNextAlarm();            // Consume delayed alarm ..
    return secsToNextAlarm();   // ... until we either have no more or valid one found
}
