#ifndef DUMMYSAPP_H
#define DUMMYSAPP_H

#include <QObject>

class DummySapp : public QObject
{
    Q_OBJECT
signals:
    void messageReceived();
};

#endif
