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
