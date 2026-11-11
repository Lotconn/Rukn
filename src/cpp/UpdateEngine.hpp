#pragma once

#include <QNetworkReply>
#include <QQmlEngine>

#include <AppEngine.hpp>

#define uEngine App::UpdateEngine::instance()

namespace App
{

class UpdateEngine : public AppBase, public AppEngine<UpdateEngine>
{
    Q_OBJECT
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool checkUpdates MEMBER m_checkUpdates NOTIFY checkUpdatesChanged)

  public:
    static auto create(QQmlEngine *qmlEngine, QJSEngine *jsEngine) -> UpdateEngine *;
    static auto logCategory() noexcept -> const QLoggingCategory &;

    auto updateCheck() const noexcept -> void;

  protected:
    explicit UpdateEngine(QObject *parent = nullptr);

  private:
    Q_DISABLE_COPY_MOVE(UpdateEngine)

    bool m_checkUpdates{};

    inline static const QUrl apiUrl{"https://munadi.org/"_L1};

    QNetworkAccessManager *netMan;

    auto connectToNetMan() const noexcept -> void;
    auto defaultConfigs() noexcept -> void override;
    auto loadConfigs() noexcept -> void override;
    auto saveConfigs() noexcept -> void override;

  signals:
    void checkUpdatesChanged();
    void updateAvailable(const QString &info);

  private slots:
    void onFinished(QNetworkReply *reply);
};

} // namespace App