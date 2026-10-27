#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QSocketNotifier>
#include <QStyleFactory>
#include <print>

#include <CoreEngine.hpp>
#include <SingleApplication>
#include <engine.h>

#ifdef Q_OS_WINDOWS
#include <windows.h>
#endif

#ifdef Q_OS_MACOS
#include <macos_window.h>
#endif

int main(int argc, char *argv[])
{
    SingleApplication app(argc, argv, true);

    if (app.isSecondary())
    {
#ifdef Q_OS_WINDOWS
        AllowSetForegroundWindow(DWORD(app.primaryPid()));
#endif
        app.sendMessage(QApplication::arguments().join(' ').toUtf8());
        qWarning() << "Munadi is already running.";
        qDebug() << "Primary instance PID: " << app.primaryPid();
        qDebug() << "Primary instance user: " << app.primaryUser().toLatin1().constData();

        return 0;
    }

    QApplication::setStyle(QStyleFactory::create("gnome"));
    QApplication::setWindowIcon(QIcon(":/qt/qml/Munadi/img/munadi.png"));
    QApplication::setOrganizationDomain("munadi.org");
    QApplication::setOrganizationName("munadi");
    QApplication::setApplicationName("Munadi");

    QFontDatabase::addApplicationFont(":/qt/qml/Munadi/data/NotoNaskhArabic-VariableFont.ttf");
    QFontDatabase::addApplicationFont(":/qt/qml/Munadi/data/NotoSans-VariableFont.ttf");
    QFontDatabase::addApplicationFont(":/qt/qml/Munadi/data/NotoSansBengali-VariableFont.ttf");

    QFont font{"Noto Sans, Noto Naskh Arabic, Noto Sans Bengali"};
    QApplication::setFont(font);

    qDebug() << "System Locale: " << QLocale::system().name();

    QQmlApplicationEngine qae;

    // These call is necessary for making `core` available before connection
    qae.singletonInstance<Munadi::CoreEngine *>("Munadi", "CoreEngine");

    if (const auto conn = QObject::connect(&app, &SingleApplication::receivedMessage, core,
                                           &Munadi::CoreEngine::onReceivedMessage);

        conn == nullptr)
        qWarning() << "Connection not established with SingleApplication";
    else
        qDebug() << "Connection established with SingleApplication";

    Engine engine;

    if (argc > 1)
    {
        if (strcmp(argv[1], "--hidden") == 0)
            engine.autostart_hidden(true);
        else if (strcmp(argv[1], "--fullscreen") == 0)
            engine.fullscreen(true);
        else if (strcmp(argv[1], "--help") == 0)
        {
            std::print("{}", qPrintable(engine.show_help(true)));
            exit(0);
        }
    }

    qae.rootContext()->setContextProperty("engine", &engine);
    qae.rootContext()->setContextProperty("updater", engine.updater);
    qae.loadFromModule("Munadi", "Main");

#ifdef Q_OS_MACOS
    if (auto *window = qobject_cast<QQuickWindow *>(qae.rootObjects().first()))
        makeTitleBarTransparent(window);
#endif

    return QApplication::exec();
}
