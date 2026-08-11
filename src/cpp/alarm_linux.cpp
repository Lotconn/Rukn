#include <QDateTime>
#include <QDebug>

#include <map>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <thread>
#include <unistd.h>

#include <alarm.h>

class AlarmLinux : public Alarm {
  int epoll_fd = -1;

  struct Meta {
    QString id;
    qint64 interval = 0;
  };

  std::map<uint64_t, Meta> entries;
  std::thread event_loop;

public:
  AlarmLinux(QObject *parent) : Alarm(parent) {
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
      qDebug() << "epoll_create1 failed";
      exit(EXIT_FAILURE);
    }

    event_loop = std::thread([&] {
      struct epoll_event ev{};
      uint64_t tmp;

      while (auto nfds = epoll_wait(epoll_fd, &ev, 1, -1)) {
        if (nfds == -1) {
          qDebug() << "epoll_wait error: " << errno;
          perror(0);
          continue;
        }

        auto res = read(ev.data.fd, &tmp, sizeof(uint64_t));

        if (res == sizeof(uint64_t)) {
          emit trigger(entries[ev.data.fd].id);
        } else if (res == -1) {
          if (errno == ECANCELED) {
            qDebug() << "read returned ECANCELED, time probably changed!";
            clear_all();
            emit trigger("alarms_reset");
          } else {
            qDebug() << "read error: " << errno << ", res: " << res;
            perror(0);
          }
        }
      }
    });
    event_loop.detach();
  }

  ~AlarmLinux() override {
    clear_all();
    close(epoll_fd);
  }

  void add(QString id, QDateTime dt, qint64 interval) override {
    if (auto epoch = dt.toSecsSinceEpoch();
        epoch > QDateTime::currentSecsSinceEpoch()) {
      auto timer_fd = timerfd_create(CLOCK_REALTIME, 0);
      if (timer_fd == -1) {
        qDebug() << "Failed to create timerfd {id: " << id << ", dt: " << dt
                 << ", interval: " << interval << "}";
        return;
      }

      struct itimerspec new_val{};

      new_val.it_value.tv_sec = epoch;
      new_val.it_interval.tv_sec = interval;

      if (timerfd_settime(timer_fd, TFD_TIMER_ABSTIME | TFD_TIMER_CANCEL_ON_SET,
                          &new_val, 0) == -1) {
        qDebug() << "timerfd_settime failed {timer_fd: " << timer_fd;
        close(timer_fd);
        return;
      }

      struct epoll_event ev{};
      ev.data.fd = timer_fd;

      if (interval)
        ev.events = EPOLLIN | EPOLLET;
      else
        ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;

      if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, timer_fd, &ev) == -1) {
        qDebug() << "epoll_ctl failed {timer_fd: " << timer_fd;
        close(timer_fd);
        return;
      }

      entries[timer_fd] = Meta{id, interval};

      qDebug() << "New alarm " << "{id: " << id << ", dt: " << dt
               << ", interval: " << interval << ", count: " << entries.size()
               << ", timer_fd: " << timer_fd << "}";
    }
  }

  void clear(QString id) override {
    for (auto it = std::begin(entries); it != std::end(entries);) {
      if (it->second.id == id) {
        close(it->first);
        it = entries.erase(it);
      } else {
        ++it;
      }
    }
  }

  void clear_all() override {
    for (auto it = std::begin(entries); it != std::end(entries); ++it)
      close(it->first);
    entries.clear();

    qDebug() << "All alarms cleared";
  }
};

Alarm *createAlarm(QObject *parent) { return new AlarmLinux(parent); }
