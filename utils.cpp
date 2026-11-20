#include "utils.h"
#include <QtMath>

static constexpr double BYTES_PER_KB = 1024.0;
static constexpr double BYTES_PER_MB = BYTES_PER_KB * 1024.0;
static constexpr double DECIMAL_THRESHOLD = 20.0;

QString Utils::formatBytes(qint64 bytes)
{
    if (bytes < BYTES_PER_KB) return QString("%1 B").arg(bytes);
    const bool isKb = bytes < BYTES_PER_MB;
    const double value = isKb ? bytes / BYTES_PER_KB : bytes / BYTES_PER_MB;
    const char *unit = isKb ? "KB" : "MB";
    if (value < DECIMAL_THRESHOLD) return QString("%1 %2") .arg(value, 0, 'f', 1) .arg(unit);
    return QString("%1 %2") .arg(qRound(value)) .arg(unit);
}

