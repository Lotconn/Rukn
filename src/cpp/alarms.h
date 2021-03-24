#ifndef ALARMS_H
#define ALARMS_H

#include <QObject>
#include <QDateTime>
#include <set>

class Alarms : public QObject
{
    Q_OBJECT

    std::set<QDateTime> entries;
    Alarms(QObject *parent = nullptr);
    void prepNextAlarm();
    qint64 secsToNextAlarm();
    static void sighandler(int);

public:
    static Alarms& instance();
    void add(QDateTime&&);
    void clear();

signals:
    void trigger();
    void empty();

};

#endif // ALARMS_H
