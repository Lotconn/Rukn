#include "updater.h"
#include "engine.h"

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QUrlQuery>
#include <QUrl>
#include <QSettings>
#include <QtDebug>

Updater::Updater(QObject *parent) :
    QObject(parent)
{
}

void Updater::check()
{
    QUrlQuery jadidParams;
#if defined Q_OS_WIN
    jadidParams.addQueryItem("os","win");
#elif defined Q_OS_MACOS
    jadidParams.addQueryItem("os","osx");
#elif defined Q_OS_ANDROID
    jadidParams.addQueryItem("os","android");
#elif defined Q_OS_LINUX
    jadidParams.addQueryItem("os","linux");
#endif

#if defined ARABIC
    jadidParams.addQueryItem("lang","ar");
#else
    jadidParams.addQueryItem("lang","en");
#endif

    jadidParams.addQueryItem("v", APP_VERSION);

    QUrl jadid(QSettings().value("App/updateUrl", MUNADI_API_URL).toString() + "jadid");
    jadid.setQuery(jadidParams);

    qDebug() << jadid.toString();

    QNetworkRequest req;
    req.setUrl(jadid);

    QNetworkAccessManager *am = new QNetworkAccessManager(this);
    connect(am, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
    am->get(req);
}

void Updater::replyFinished(QNetworkReply *reply)
{
    QString replyString = QString(reply->readAll());

    if(reply->error() == 0 && replyString.length())
    {
        emit updateAvailable(replyString);
    }

    reply->deleteLater();
}
