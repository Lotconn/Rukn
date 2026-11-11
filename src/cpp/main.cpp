#include <QApplication>
#include <QFontDatabase>
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
    // using Qt::StringLiterals::operator""_s;
    using Qt::StringLiterals::operator""_L1;

    SingleApplication app(argc, argv, true);

    if (app.isSecondary())
    {
#ifdef Q_OS_WINDOWS
        AllowSetForegroundWindow(DWORD(app.primaryPid()));
#endif
        app.sendMessage(QApplication::arguments().join(' ').toUtf8());
        qWarning() << "App is already running.";
        qDebug() << "Primary instance PID: " << app.primaryPid();
        qDebug() << "Primary instance user: " << app.primaryUser().toLatin1().constData();

        return 0;
    }

    QApplication::setApplicationName("Munadi"_L1);
    QApplication::setDesktopFileName("org.munadi.Munadi"_L1);
    QApplication::setOrganizationDomain("munadi.org"_L1);
    QApplication::setOrganizationName("munadi"_L1);
    QApplication::setStyle(QStyleFactory::create("gnome"_L1));
    QApplication::setWindowIcon(QIcon{":/qt/qml/App/img/munadi.png"_L1});

    QFontDatabase::addApplicationFont(":/qt/qml/App/data/NotoNaskhArabic-VariableFont.ttf"_L1);
    QFontDatabase::addApplicationFont(":/qt/qml/App/data/NotoSans-VariableFont.ttf"_L1);
    QFontDatabase::addApplicationFont(":/qt/qml/App/data/NotoSansBengali-VariableFont.ttf"_L1);

    const QFont font{"Noto Sans, Noto Naskh Arabic, Noto Sans Bengali"_L1};
    QApplication::setFont(font);

    qDebug() << "System Locale: " << QLocale::system().name();

    QQmlApplicationEngine qae;

    // These call is necessary for making `core` available before connection
    qae.singletonInstance<App::CoreEngine *>("App"_L1, "CoreEngine"_L1);

    if (const auto conn = QObject::connect(&app, &SingleApplication::receivedMessage, core,
                                           &App::CoreEngine::onReceivedMessage);

        conn == nullptr)
        qWarning() << "Connection not established with SingleApplication";
    else
        qDebug() << "Connection established with SingleApplication";

    Engine engine;

    if (argc > 1)
    {
        if (strcmp(argv[1], "--hidden") == 0)
            engine.start_hidden(true);
        else if (strcmp(argv[1], "--fullscreen") == 0)
            engine.fullscreen(true);
        else if (strcmp(argv[1], "--help") == 0)
        {
            std::print("{}", qPrintable(engine.show_help(true)));
            exit(0);
        }
    }

    qae.rootContext()->setContextProperty("engine"_L1, &engine);
    qae.loadFromModule("App"_L1, "Main"_L1);

#ifdef Q_OS_MACOS
    if (auto *window = qobject_cast<QQuickWindow *>(qae.rootObjects().first()))
        makeTitleBarTransparent(window);
#endif

    return QApplication::exec();
}
