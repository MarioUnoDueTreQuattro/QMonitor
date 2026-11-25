#include "utils.h"
#include <QtMath>
#include <QList>

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


QList<int> Utils::humanDivisors(int range, int minDiv, int maxDiv, bool allowHalfSteps)
{
    QList<int> result;
    const int scaled = allowHalfSteps ? range * 2 : range;
    for (int n = minDiv; n <= maxDiv; ++n)
    {
        if (scaled % n == 0)
        {
            result.append(n);
        }
    }
    return result;
}