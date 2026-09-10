#pragma once

#include <QLoggingCategory>
#include <QTimer>

#include <CoreEngine.hpp>

Q_DECLARE_LOGGING_CATEGORY(coreEngine)
Q_DECLARE_LOGGING_CATEGORY(multimediaEngine)
Q_DECLARE_LOGGING_CATEGORY(prayerEngine)
Q_DECLARE_LOGGING_CATEGORY(uiEngine)
Q_DECLARE_LOGGING_CATEGORY(common)

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define logDebug(logCategory) qCDebug(logCategory).noquote().nospace() << __func__ << ": debug: "_L1
#define logInfo(logCategory)  qCInfo(logCategory).noquote().nospace() << __func__ << ": info: "_L1
#define logWarning(logCategory)                                                                    \
    qCWarning(logCategory).noquote().nospace() << __func__ << ": warning: "_L1
#define logCritical(logCategory)                                                                   \
    qCCritical(logCategory).noquote().nospace() << __func__ << ": error: "_L1
#define logFatal(logCategory) qCFatal(logCategory).noquote().nospace() << __func__ << ": fatal: "_L1
// NOLINTEND(cppcoreguidelines-macro-usage)

// Allows shorthand QLatin1StringView ""_L1 operators
// Included in this internal header for all sources to utilize
using Qt::StringLiterals::operator""_L1; // NOLINT(*global-names-in-headers)

auto isValidEntry(const QVariantMap &entryMaps, const qsizetype &refEntrySize,
                  const QMap<QString, int> &refFields, const qsizetype &refMapSize = 0) -> bool;
auto isValidEntry(const QVariantList &entryList, const qsizetype &refMapSize,
                  const QMap<QString, int> &refFields, const qsizetype &refListSize = 0) -> bool;
auto isValidMap(const QVariantMap &entry, const qsizetype &refMapSize,
                const QMap<QString, int> &refFields) -> bool;
auto textsRefresherReal() -> int;