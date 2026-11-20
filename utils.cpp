#include "utils.h"
#include <QtMath>

static constexpr double BYTES_PER_KB = 1024.0;
static constexpr double BYTES_PER_MB = BYTES_PER_KB * 1024.0;
static constexpr double DECIMAL_THRESHOLD = 20.0;

QString Utils::formatBytes(qint64 bytes)
{
    if (bytes < BYTES_PER_KB) return QString::number(bytes) + " B";
    const bool isKb = bytes < BYTES_PER_MB;
    const double value = isKb ? bytes / BYTES_PER_KB : bytes / BYTES_PER_MB;
    if (value < DECIMAL_THRESHOLD)
    {
        const int rounded10 = qRound(value * 10.0);
        if ((rounded10 % 10) == 0) return QString::number(rounded10 / 10) + (isKb ? " KB" : " MB");
        return QString::number(value, 'f', 1) + (isKb ? " KB" : " MB");
    }
    return QString::number(qRound(value)) + (isKb ? " KB" : " MB");
}
