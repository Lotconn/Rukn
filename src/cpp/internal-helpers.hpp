#pragma once

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(coreEngine)
Q_DECLARE_LOGGING_CATEGORY(prayerEngine)
Q_DECLARE_LOGGING_CATEGORY(uiEngine)

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define logDebug(logCategory)                                                                      \
    qCDebug(logCategory).noquote().nospace() << __func__ << ": debug : "_L1
#define logInfo(logCategory) qCInfo(logCategory).noquote().nospace() << __func__ << ": info : "_L1
#define logWarning(logCategory)                                                                    \
    qCWarning(logCategory).noquote().nospace() << __func__ << ": warning : "_L1
#define logCritical(logCategory)                                                                   \
    qCCritical(logCategory).noquote().nospace() << __func__ << ": error : "_L1
#define logFatal(logCategory)                                                                      \
    qCFatal(logCategory).noquote().nospace() << __func__ << ": fatal : "_L1
// NOLINTEND(cppcoreguidelines-macro-usage)

// Allows shorthand QLatin1StringView ""_L1 operators
// Included in this internal header for all sources to utilize
using Qt::StringLiterals::operator""_L1;

[[nodiscard]] auto isValidAudioEntry(const QVariantMap &entry) -> bool;
auto               textsRefresherReal() -> int;