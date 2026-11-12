#include <engine.h>

#include <QDir>
#include <QTextStream>

#if defined(Q_OS_WIN)

#include <windows.h>

#endif

using namespace Qt::StringLiterals;

QString Engine::show_help(bool /*flag*/)
{
    return "\nUsage: rukn [options]\n"
           "Options:\n"
           "  --hidden        Start minimized to system tray\n"
           "  --fullscreen    Start in fullscreen mode\n"
           "  --help          Show this help message and exit\n\n";
}

void Engine::start_hidden(bool flag)
{ _hidden = flag; }

bool Engine::start_hidden()
{ return _hidden; }

void Engine::fullscreen(bool flag)
{ _fullscreen = flag; }

bool Engine::fullscreen()
{ return _fullscreen; }

QString Engine::getWhatsNew()
{
    QFile whats_new(":/qt/qml/App/data/whatsnew.html");
    std::ignore = whats_new.open(QFile::ReadOnly | QFile::Text);
    QTextStream ts(&whats_new);

    QString out = ts.readAll();

    return out;
}
