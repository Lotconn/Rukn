#ifndef ALARMS_H
#define ALARMS_H

#include <QObject>
#include <QDateTime>

class Alarm : public QObject
{
    Q_OBJECT

public:
    Alarm(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~Alarm() = default;

    Q_INVOKABLE virtual void add(QString id, QDateTime dt, qint64 interval = 0) = 0;
    Q_INVOKABLE virtual void clear(QString id) = 0;
    Q_INVOKABLE virtual void clear_all() = 0;

signals:
    void trigger(QString id);
    void empty();
};

Alarm *createAlarm(QObject *parent = nullptr);

#endif
