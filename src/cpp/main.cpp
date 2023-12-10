#include <QQmlContext>
#include <QApplication>
#include <QQmlApplicationEngine>

#ifndef ANDROID
#include <kdsingleapplication.h>
#endif

#include "engine.h"
#include "alarms.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Engine engine;
    Alarms alarms;

#ifndef ANDROID
    KDSingleApplication sapp;

    if (!sapp.isPrimaryInstance()) {
        sapp.sendMessage(nullptr);
        qDebug() << "Munadi already running!";

        return 0;
    }
#endif

    app.setOrganizationDomain("munadi.org");
    app.setApplicationName("Munadi");

    QFontDatabase::addApplicationFont(":/data/NotoNaskhArabicUI-Regular.ttf");
    QFontDatabase::addApplicationFont(":/data/NotoNaskhArabicUI-Bold.ttf");
    QFontDatabase::addApplicationFont(":/data/NotoSans-Regular.ttf");
    QFontDatabase::addApplicationFont(":/data/NotoSans-Bold.ttf");

    QFont font {"Noto Sans, Noto Naskh Arabic UI"};
    app.setFont(font);

    auto locale = QLocale::system().name();
    qDebug() << "Locale: " << locale;

    if(locale.startsWith("en")) // Work around for Android
        QLocale::setDefault(QLocale::English);

    QTranslator translator;
    // look up e.g. :/i18n/munadi_ar.qm
    if (translator.load(QLocale(), QLatin1String("munadi"), QLatin1String("_"), QLatin1String(":/i18n")))
        app.installTranslator(&translator);

    QQmlApplicationEngine qae;

    if(argc > 1 && strcmp(argv[1], "--hidden") == 0)
    {
        engine.autostart_hidden(true);
    }

    qae.rootContext()->setContextProperty("engine", &engine);
    qae.rootContext()->setContextProperty("alarms", &alarms);
#ifndef ANDROID
    qae.rootContext()->setContextProperty("sapp", &sapp);
#endif
    //qae.rootContext()->setContextProperty("updater", engine.updater);
    qae.load(QUrl(QLatin1String("qrc:/qml/main.qml")));

    return app.exec();
}
