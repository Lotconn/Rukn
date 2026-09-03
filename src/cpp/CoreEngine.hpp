#pragma once

#include <QObject>
#include <QReadWriteLock>
#include <QTranslator>
#include <fileman>

namespace Munadi
{

class CoreEngine : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(QVariantList audioList MEMBER m_audioList WRITE setAudioList NOTIFY audioListChanged)
    Q_PROPERTY(QVariantList prayerConfig MEMBER m_prayerConfig WRITE setPrayerConfig NOTIFY
                   prayerConfigChanged)

  signals:
    auto audioListChanged(const QVariantList &audioList) -> void;
    auto localeChanged() -> void;
    auto prayerConfigChanged(const QVariantList &prayerConfig) -> void;

  public:
    explicit CoreEngine(QObject *parent = nullptr);
    ~CoreEngine() override;

    static auto instance() -> CoreEngine &;

    auto applyLocale() -> void;

    Q_INVOKABLE void addAthan(const QUrl &fileUrl, int fileDialogCategory);
    Q_INVOKABLE void modifyPrayerConfig(int prayerIndex, const QString &field,
                                        const QVariant &value);
    Q_INVOKABLE void removeAthan(int selectedIndex);

  protected:
    bool eventFilter(QObject *object, QEvent *event) override;

  private:
    Q_DISABLE_COPY_MOVE(CoreEngine);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Variables
    ////////////////////////////////////////////////////////////////////////////////////////////////

    static CoreEngine *s_singletonInstance;

    FileMan fileman;

    QVariantList m_audioList{
        QVariantMap{   {"name", "Makkah (built-in)"},
                    {"path", "qrc:/qt/qml/Munadi/data/athan-makkah.ogg"}  },
        QVariantMap{{"name", "Ar-Rashid (built-in)"},
                    {"path", "qrc:/qt/qml/Munadi/data/athan-arRashid.ogg"}},
        QVariantMap{     {"name", "Fajr (built-in)"},
                    {"path", "qrc:/qt/qml/Munadi/data/fajr-athan.ogg"}    }
    };

    qsizetype defaultAudioCount{this->m_audioList.size()};

    quint8 defaultAthan{0};
    quint8 defaultFajrAthan{2};
    quint8 defaultMaghribAthan{1};

    std::float_t defaultVolume{1.0}; // 0.0 - 1.0
    std::float_t defaultFajrVolume{0.2};
    std::float_t defaultIshaVolume{0.5};

    QVariantList m_prayerConfig = {
        QVariantMap{{"adjustment", 0},
                    {"volume", defaultFajrVolume},
                    {"athan_idx", defaultFajrAthan}                                               }, // [0] Fajr
        QVariantMap{{"adjustment", 0},
                    {"volume", defaultVolume},
                    {"athan_idx", defaultAthan}                                                   }, // [1] Sunrise
        QVariantMap{
                    {"adjustment", 0},      {"volume", defaultVolume}, {"athan_idx", defaultAthan}}, // [2] Dhuhr
        QVariantMap{
                    {"adjustment", 0},      {"volume", defaultVolume}, {"athan_idx", defaultAthan}}, // [3] Asr
        QVariantMap{{"adjustment", 0},
                    {"volume", defaultVolume},
                    {"athan_idx", defaultMaghribAthan}                                            }, // [4] Maghrib
        QVariantMap{{"adjustment", 0},
                    {"volume", defaultIshaVolume},
                    {"athan_idx", defaultAthan}                                                   }  // [5] Isha
    };

    QTranslator m_translator;

    QReadWriteLock varLock;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Functions
    ////////////////////////////////////////////////////////////////////////////////////////////////

    auto setAudioList(const QVariantList &audioList) -> void;
    auto setPrayerConfig(const QVariantList &prayerConfig) -> void;
};

} // namespace Munadi