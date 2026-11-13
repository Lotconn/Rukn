#pragma once

#include <QDateTime>
#include <QObject>

class Alarm : public QObject {
  Q_OBJECT

public:
  explicit Alarm(QObject *parent = nullptr) : QObject(parent) {}
  ~Alarm() override = default;

  Q_INVOKABLE virtual void add(QString id, QDateTime dt, qint64 interval = 0) = 0;
  Q_INVOKABLE virtual void clear(QString id)                                  = 0;
  Q_INVOKABLE virtual void clear_all()                                        = 0;

signals:
  void trigger(QString id);
  void empty();
};

Alarm *createAlarm(QObject *parent = nullptr);
