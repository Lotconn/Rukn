#include <QDebug>
#include <QDateTime>

#include <unistd.h>
#include <signal.h>
#include <chrono>
#include <cstring>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <map>
#include <thread>

#include <alarm.h>

class AlarmMacos : public Alarm
{
    int kq_fd = -1;
    uint64_t next_ident = 1;

    struct Meta {
        QString id;
        qint64 interval = 0;
    };

    std::map<uint64_t, Meta> entries;
    std::thread event_loop;

public:
    AlarmMacos(QObject *parent) : Alarm(parent)
    {
        kq_fd = kqueue();
        if(kq_fd == -1)
        {
            qDebug() << "kqueue failed";
            exit(EXIT_FAILURE);
        }

        event_loop = std::thread([&]{
            struct kevent ev;
            struct kevent heartbeat;

            EV_SET(&heartbeat, 0, EVFILT_TIMER, EV_ADD, NOTE_SECONDS, 2, nullptr);
            kevent(kq_fd, &heartbeat, 1, nullptr, 0, nullptr);

            auto last_heartbeat = std::chrono::steady_clock::now();

            while (true)
            {
                auto n = kevent(kq_fd, nullptr, 0, &ev, 1, nullptr);

                if(n == -1)
                {
                    if(errno == EINTR)
                        continue;
                    qDebug() << "kevent error:" << errno;
                    perror(0);
                    continue;
                }

                if(n == 0)
                    continue;

                auto ident = ev.ident;

                if(ident == 0)
                {
                    auto now = std::chrono::steady_clock::now();
                    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_heartbeat).count();
                    last_heartbeat = now;

                    if(elapsed > 3)
                    {
                        qDebug() << "Sleep detected via heartbeat (" << elapsed << "s gap)";
                        clear_all();
                        emit trigger("alarms_reset");
                    }
                    continue;
                }

                auto it = entries.find(ident);
                if(it == entries.end())
                    continue;

                emit trigger(it->second.id);

                if(it->second.interval > 0)
                {
                    struct kevent kev;
                    EV_SET(&kev, ident, EVFILT_TIMER, EV_ADD | EV_ONESHOT, NOTE_SECONDS,
                           it->second.interval, nullptr);
                    if(kevent(kq_fd, &kev, 1, nullptr, 0, nullptr) == -1)
                        qDebug() << "Failed to re-add repeating timer for id:" << it->second.id
                                 << ", errno: " << errno << " (" << strerror(errno) << ")";
                }
                else
                {
                    entries.erase(ident);
                }
            }
        });
        event_loop.detach();
    }

    ~AlarmMacos() override
    {
        clear_all();
        close(kq_fd);
    }

    void add(QString id, QDateTime dt, qint64 interval) override
    {
        if(auto epoch = dt.toSecsSinceEpoch(); epoch > QDateTime::currentSecsSinceEpoch())
        {
            auto ident = next_ident++;
            auto secs_until = epoch - QDateTime::currentSecsSinceEpoch();

            struct kevent kev;
            EV_SET(&kev, ident, EVFILT_TIMER, EV_ADD | EV_ONESHOT, NOTE_SECONDS,
                   secs_until, nullptr);

            if(kevent(kq_fd, &kev, 1, nullptr, 0, nullptr) == -1)
            {
                qDebug() << "Failed to add timer {id: " << id << ", ident: " << ident
                         << ", errno: " << errno << " (" << strerror(errno) << ")}";
                return;
            }

            entries[ident] = Meta{id, interval};

            qDebug() << "New alarm " << "{id: " << id << ", dt: " << dt << ", interval: "
                     << interval << ", count: " << entries.size() << ", ident: " << ident << "}";
        }
    }

    void clear(QString id) override
    {
        struct kevent kev;

        for(auto it = std::begin(entries); it != std::end(entries);)
        {
            if(it->second.id == id)
            {
                EV_SET(&kev, it->first, EVFILT_TIMER, EV_DELETE, 0, 0, nullptr);
                kevent(kq_fd, &kev, 1, nullptr, 0, nullptr);
                it = entries.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    void clear_all() override
    {
        struct kevent kev;

        for(auto it = std::begin(entries); it != std::end(entries); ++it)
        {
            EV_SET(&kev, it->first, EVFILT_TIMER, EV_DELETE, 0, 0, nullptr);
            kevent(kq_fd, &kev, 1, nullptr, 0, nullptr);
        }
        entries.clear();

        qDebug() << "All alarms cleared";
    }
};

Alarm *createAlarm(QObject *parent)
{
    return new AlarmMacos(parent);
}
