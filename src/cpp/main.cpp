#include <QSettings>
#include <QQmlContext>
#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "engine.h"

int main(int argc, char *argv[])
{

#if defined(Q_OS_ANDROID) && defined(Q_OS_LINUX)
    //qputenv("QT_QUICK_CONTROLS_STYLE", "Material");
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QGuiApplication app(argc, argv);

    app.setOrganizationDomain("munadi.org");
    app.setApplicationName("Munadi");

    QFontDatabase::addApplicationFont("qrc:/data/NotoNaskhArabicUI-Regular.ttf");
    QFontDatabase::addApplicationFont("qrc:/data/NotoNaskhArabicUI-Bold.ttf");
    QFontDatabase::addApplicationFont("qrc:/data/NotoSans-Regular.ttf");
    QFontDatabase::addApplicationFont("qrc:/data/NotoSans-Bold.ttf");

    QFont font {"Noto Naskh Arabic UI, Noto Sans"};
    app.setFont(font);

    QTranslator translator;
    // look up e.g. :/i18n/munadi_ar.qm
    if (translator.load(QLocale(), QLatin1String("munadi"), QLatin1String("_"), QLatin1String(":/i18n")))
        app.installTranslator(&translator);

    QQmlApplicationEngine qae;

    Engine engine;

    if(argc > 1 && strcmp(argv[1], "--hidden") == 0)
    {
        engine.setStartupFlag();
    }

    qae.rootContext()->setContextProperty("engine", &engine);
    //qae.rootContext()->setContextProperty("updater", engine.updater);
    qae.load(QUrl(QLatin1String("qrc:/qml/main.qml")));

    return app.exec();
}
