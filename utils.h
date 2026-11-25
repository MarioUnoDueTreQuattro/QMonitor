#ifndef UTILS_H
#define UTILS_H

#include <QString>

class Utils
{
public:
    static QString formatBytes(qint64 bytes);
    static QList<int> humanDivisors(int range, int minDiv = 2, int maxDiv = 25, bool allowHalfSteps = true);
};

#endif // UTILS_H
