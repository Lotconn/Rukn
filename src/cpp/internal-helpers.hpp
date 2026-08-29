#pragma once

#include <PrayerEngine.hpp>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(munadi)

#define logDebug    qCDebug(munadi).noquote().nospace() << __func__ << ": debug : "_L1
#define logInfo     qCInfo(munadi).noquote().nospace() << __func__ << ": info : "_L1
#define logWarning  qCWarning(munadi).noquote().nospace() << __func__ << ": warning : "_L1
#define logCritical qCCritical(munadi).noquote().nospace() << __func__ << ": error : "_L1

// Allows shorthand QLatin1StringView ""_L1 operators
// Included in this internal header for all sources to utilize
using Qt::StringLiterals::operator""_L1;