#ifndef CPUREADER_H
#define CPUREADER_H

#include <QtGlobal>

// Reads the total CPU load from /proc/stat.
// Keeps the previous counters, so the result is the average load
// between two consecutive calls (works with any sampling interval).
class CpuReader
{
public:
    CpuReader();

    // Returns 0..100. The first call returns 0 (no previous reading yet).
    double readLoadPercent();

private:
    quint64 m_previousTotal;
    quint64 m_previousIdle;
    bool m_hasPrevious;
};

#endif // CPUREADER_H