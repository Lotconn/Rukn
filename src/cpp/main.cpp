#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSocketNotifier>
#include <QStyleFactory>

#ifdef Q_OS_LINUX
#include <kdsingleapplication.h>
#endif

#include <QQuickWindow>

#include <alarm.h>
#include <engine.h>

#ifdef Q_OS_MACOS
#include <macos_window.h>
#endif

#ifndef Q_OS_LINUX
#include <dummysapp.h>
#endif

#ifdef Q_OS_LINUX
#include <sys/socket.h>
#include <signal.h>
#include <unistd.h>

static int sigFd[2] = {-1, -1};

static void unixSignalHandler(int)
{
    char a = 'A';
    [[maybe_unused]] auto trash = ::write(sigFd[1], &a, sizeof(a));
    // just to avoid compiler warnings
}
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

    app.setWindowIcon(QIcon(":/qt/qml/Munadi/img/munadi.png"));
    app.setOrganizationDomain("munadi");
    app.setApplicationName("Munadi");

    QFontDatabase::addApplicationFont(":/qt/qml/Munadi/data/NotoNaskhArabicUI-Regular.ttf");
    QFontDatabase::addApplicationFont(":/qt/qml/Munadi/data/NotoNaskhArabicUI-Bold.ttf");
    QFontDatabase::addApplicationFont(":/qt/qml/Munadi/data/NotoSans-ExtraLight.ttf");
    QFontDatabase::addApplicationFont(":/qt/qml/Munadi/data/NotoSans-Regular.ttf");
    QFontDatabase::addApplicationFont(":/qt/qml/Munadi/data/NotoSans-Bold.ttf");

    QFont font {"Noto Sans, Noto Naskh Arabic UI"};
    app.setFont(font);

    qDebug() << "System Locale: " << QLocale::system().name();

    QTranslator translator;
    if(QLocale l(QLocale::system()); l.language() == QLocale::Arabic) {
        if (translator.load(l,
                            QLatin1String("munadi"),
                            QLatin1String("_"),
                            QLatin1String(":/qt/qml/Munadi/i18n"))) {
            app.installTranslator(&translator);
            qDebug() << "Loaded translator file: " << translator.filePath();
        }
    } else if (l.name() != QLatin1String("en_US")) {
        QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
        qInfo() << "Setting locale to: " << QLocale().name();
    }

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

    qae.rootContext()->setContextProperty("alarm", alarm);
    qae.rootContext()->setContextProperty("engine", &engine);
#ifdef Q_OS_LINUX
    qae.rootContext()->setContextProperty("sapp", &sapp);
#else
    DummySapp dummySapp;
    qae.rootContext()->setContextProperty("sapp", &dummySapp);
#endif
    qae.rootContext()->setContextProperty("updater", engine.updater);
    qae.load(QUrl(QLatin1String("qrc:/qt/qml/Munadi/qml/main.qml")));

#ifdef Q_OS_MACOS
    if(auto *window = qobject_cast<QQuickWindow *>(qae.rootObjects().first()))
        makeTitleBarTransparent(window);
#endif

#ifdef Q_OS_LINUX
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigFd) != 0)
        qFatal("Failed to create signal socket pair");

    QSocketNotifier sigNotifier(sigFd[0], QSocketNotifier::Read);
    QObject::connect(&sigNotifier, &QSocketNotifier::activated, &app, [](QSocketDescriptor) {
        char tmp;
        [[maybe_unused]] auto trash = ::read(sigFd[0], &tmp, sizeof(tmp));
        // just to avoid compiler warnings
        QCoreApplication::quit();
    });

    struct sigaction sa {};
    sa.sa_handler = unixSignalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
#endif

    int ret = app.exec();

#ifdef Q_OS_LINUX
    ::close(sigFd[0]);
    ::close(sigFd[1]);
#endif

    return ret;
}
