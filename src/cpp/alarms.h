#ifndef ALARMS_H
#define ALARMS_H

#include <QObject>
#include <QDateTime>
#include <map>
#include <vector>
#include <optional>

class Alarms : public QObject
{
    Q_OBJECT

    struct Meta {
        QString id;
        qint64 interval;
    };

    //std::set<qint64> entries;
    std::map<qint64, std::vector<Meta>> entries;
    Alarms(QObject *parent = nullptr);
    void prepNextAlarm();
    qint64 secsToNextAlarm();
    static void sighandler(int);

public:
    static Alarms& instance();
    Q_INVOKABLE void add(QDateTime, QString id, qint64 interval = 0);
    Q_INVOKABLE void clear(QString id);

signals:
    void trigger(QString id);
    void empty();

};

#endif // ALARMS_H
