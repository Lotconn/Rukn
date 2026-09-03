#include <QVariant>

#include "internal-helpers.hpp"

bool isValidAudioEntry(const QVariantMap &entry)
{
    return entry.contains("name") && entry.contains("path") &&
           entry.value("name").typeId() == QMetaType::QString &&
           entry.value("path").typeId() == QMetaType::QString;
}

int textsRefresherReal()
{ return 0; }