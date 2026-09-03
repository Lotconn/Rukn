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
    Q_PROPERTY(QStringList themeList MEMBER m_themeList NOTIFY themeListChanged)
    Q_PROPERTY(
        int themeListIndex READ themeListIndex WRITE setThemeListIdx NOTIFY themeListIdxChanged)

  signals:
    void themeListChanged();
    void themeListIdxChanged(int);

  public:
    explicit UiEngine(QObject *parent = nullptr);
    ~UiEngine() override;

    static UiEngine &instance();

    auto themeList() -> QStringList;
    auto themeListIndex() -> int;
    auto setThemeListIdx(int) -> void;

  private:
    Q_DISABLE_COPY_MOVE(UiEngine);

    static UiEngine *s_singletonInstance;

    int         m_themeListIdx = 0;
    QStringList m_themeList;

  private slots: // NOLINT(readability-redundant-access-specifiers)
    auto initTexts() -> void;
};

} // namespace Munadi