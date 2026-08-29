#pragma once

#include <QObject>
#include <QQmlEngine>

namespace Munadi
{
class UiEngine : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(QStringList themeList READ themeList CONSTANT)
    Q_PROPERTY(
        int themeListIndex READ themeListIndex WRITE setThemeListIdx NOTIFY themeListIdxChanged)

  signals:
    void themeListIdxChanged(int);

  public:
    explicit UiEngine(QObject *parent = nullptr);
    ~UiEngine() override;

    static UiEngine *create(QQmlEngine *, QJSEngine *);
    static UiEngine &instance();

    QStringList themeList();
    int         themeListIndex();
    void        setThemeListIdx(int);

  private:
    Q_DISABLE_COPY_MOVE(UiEngine);

    static UiEngine *s_singletonInstance;

    int m_themeListIdx;
};
} // namespace Munadi