#ifndef ALARMS_H
#define ALARMS_H

#include <QObject>
#include <QDateTime>
#include <map>
#include <thread>

class Alarms : public QObject
{
    Q_OBJECT

    int epoll_fd = -1;


    struct Meta {
        QString id;
        struct itimerspec timer;
    };

    std::map<uint64_t, Meta> entries;
    std::thread event_loop;

public:
    Alarms(QObject *parent = nullptr);
    virtual ~Alarms();

    Q_INVOKABLE void add(QString id, QDateTime, qint64 interval = 0);
    Q_INVOKABLE void clear(QString id);
    Q_INVOKABLE void clear_all();

signals:
    void trigger(QString id);
    void empty();

};

#endif // ALARMS_H
