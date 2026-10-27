#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QSocketNotifier>
#include <QStyleFactory>

#include <CoreEngine.hpp>
#include <LocationEngine.hpp>
#include <MultimediaEngine.hpp>
#include <PrayerEngine.hpp>
#include <SingleApplication>
#include <engine.h>

#ifdef Q_OS_WINDOWS
#include <windows.h>
#endif

#ifdef Q_OS_MACOS
#include <macos_window.h>
#endif

#ifdef Q_OS_LINUX
#include <csignal>
#include <print>
#include <sys/socket.h>
#include <unistd.h>

static int sigFd[2] = {-1, -1};

static void unixSignalHandler(int)
{
    char                  a     = 'A';
    [[maybe_unused]] auto trash = ::write(sigFd[1], &a, sizeof(a));
    // just to avoid compiler warnings
}
#endif

int main(int argc, char *argv[])
{
    // qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));

    QApplication app(argc, argv);

    QApplication::setStyle(QStyleFactory::create("gnome"));

    SingleApplication sapp(argc, argv, true);
    if (sapp.isSecondary())
    {
#ifdef Q_OS_WINDOWS
        AllowSetForegroundWindow(DWORD(sapp.primaryPid()));
#endif
        sapp.sendMessage(QApplication::arguments().join(' ').toUtf8());
        qWarning() << "Munadi is already running.";
        qDebug() << "Primary instance PID: " << sapp.primaryPid();
        qDebug() << "Primary instance user: " << sapp.primaryUser().toLatin1().constData();
        return 0;
    }

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

    // These nudges are necessary for satisfying the dependencies, and should be earliest before
    // anything
    qae.singletonInstance<Munadi::CoreEngine *>("Munadi", "CoreEngine");
    qae.singletonInstance<Munadi::LocationEngine *>("Munadi", "LocationEngine");
    qae.singletonInstance<Munadi::PrayerEngine *>("Munadi", "PrayerEngine");
    qae.singletonInstance<Munadi::MultimediaEngine *>("Munadi", "MultimediaEngine");

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
    qae.rootContext()->setContextProperty("sapp", &sapp);
    qae.rootContext()->setContextProperty("updater", engine.updater);
    qae.loadFromModule("Munadi", "Main");

#ifdef Q_OS_MACOS
    if (auto *window = qobject_cast<QQuickWindow *>(qae.rootObjects().first()))
        makeTitleBarTransparent(window);
#endif

#ifdef Q_OS_LINUX
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigFd) != 0)
        qFatal("Failed to create signal socket pair");

    QSocketNotifier sigNotifier(sigFd[0], QSocketNotifier::Read);
    QObject::connect(&sigNotifier, &QSocketNotifier::activated, &app, [](QSocketDescriptor) {
        char                  tmp   = 0;
        [[maybe_unused]] auto trash = ::read(sigFd[0], &tmp, sizeof(tmp));
        // just to avoid compiler warnings
        QCoreApplication::quit();
    });

    struct sigaction sa{};
    sa.sa_handler = unixSignalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
#endif

    int ret = QApplication::exec();

#ifdef Q_OS_LINUX
    ::close(sigFd[0]);
    ::close(sigFd[1]);
#endif

    return ret;
}
