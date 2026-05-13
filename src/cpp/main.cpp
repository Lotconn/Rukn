#include <QQmlContext>
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QStyleFactory>

#ifdef Q_OS_LINUX
#include <kdsingleapplication.h>
#endif

#include <QQuickWindow>

#include "engine.h"
#include "alarm.h"

#ifdef Q_OS_MACOS
#include "macos_window.h"
#endif

#ifndef Q_OS_LINUX
#include "dummysapp.h"
#endif

int main(int argc, char *argv[])
{
    //qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));

    QApplication app(argc, argv);
    Engine engine;
    auto *alarm = createAlarm(&app);

    QApplication::setStyle(QStyleFactory::create("gnome"));
#ifdef Q_OS_LINUX
    KDSingleApplication sapp;

    if (!sapp.isPrimaryInstance()) {
        sapp.sendMessage(nullptr);
        qDebug() << "Munadi already running!";

        return 0;
    }
#endif

    app.setWindowIcon(QIcon(":/img/munadi.png"));
    app.setOrganizationDomain("munadi");
    app.setApplicationName("Munadi");

    QFontDatabase::addApplicationFont(":/data/NotoNaskhArabicUI-Regular.ttf");
    QFontDatabase::addApplicationFont(":/data/NotoNaskhArabicUI-Bold.ttf");
    QFontDatabase::addApplicationFont(":/data/NotoSans-Regular.ttf");
    QFontDatabase::addApplicationFont(":/data/NotoSans-Bold.ttf");

    QFont font {"Noto Sans, Noto Naskh Arabic UI"};
    app.setFont(font);

    auto locale = QLocale::system().name();
    qDebug() << "Locale: " << locale;

    // if(locale.startsWith("en")) // Work around for Android
    QLocale::setDefault(QLocale::English);

    QTranslator translator;
    // look up e.g. :/i18n/munadi_ar.qm
    if (translator.load(QLocale(), QLatin1String("munadi"), QLatin1String("_"), QLatin1String(":/i18n")))
        app.installTranslator(&translator);

    QQmlApplicationEngine qae;

    if(argc > 1)
    {
        if(strcmp(argv[1], "--hidden") == 0)
            engine.autostart_hidden(true);
        else if(strcmp(argv[1], "--fullscreen") == 0)
            engine.fullscreen(true);
        else if(strcmp(argv[1], "--help") == 0) {
            printf("%s", qPrintable(engine.show_help(true)));
            exit(0);
        }
    }

    qae.rootContext()->setContextProperty("engine", &engine);
    qae.rootContext()->setContextProperty("alarm", alarm);
#ifdef Q_OS_LINUX
    qae.rootContext()->setContextProperty("sapp", &sapp);
#else
    DummySapp dummySapp;
    qae.rootContext()->setContextProperty("sapp", &dummySapp);
#endif
    qae.rootContext()->setContextProperty("updater", engine.updater);
    qae.load(QUrl(QLatin1String("qrc:/qml/main.qml")));

#ifdef Q_OS_MACOS
    if(auto *window = qobject_cast<QQuickWindow *>(qae.rootObjects().first()))
        makeTitleBarTransparent(window);
#endif

    return app.exec();
}
