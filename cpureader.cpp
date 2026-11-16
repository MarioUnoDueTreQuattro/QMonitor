#include "cpureader.h"

#include <QByteArray>
#include <QFile>
#include <QList>

CpuReader::CpuReader()
    : m_previousTotal(0),
      m_previousIdle(0),
      m_hasPrevious(false)
{
}

double CpuReader::readLoadPercent()
{
    QFile statFile("/proc/stat");
    if (!statFile.open(QIODevice::ReadOnly))
        return 0.0;

    // /proc files report size 0: use readAll()
    QByteArray content = statFile.readAll();
    statFile.close();

    // First line: "cpu  user nice system idle iowait irq softirq steal guest guest_nice"
    int lineEnd = content.indexOf('\n');
    QList<QByteArray> fields = content.left(lineEnd).simplified().split(' ');
    if (fields.size() < 5 || fields.at(0) != "cpu")
        return 0.0;

    // Sum user..steal (fields 1..8). guest and guest_nice are already
    // included in user and nice, so they must not be added again.
    quint64 total = 0;
    for (int i = 1; i < fields.size() && i <= 8; ++i)
        total += fields.at(i).toULongLong();

    // Idle time = idle + iowait
    quint64 idle = fields.at(4).toULongLong();
    if (fields.size() > 5)
        idle += fields.at(5).toULongLong();

    double loadPercent = 0.0;
    if (m_hasPrevious && total > m_previousTotal)
    {
        quint64 totalDelta = total - m_previousTotal;
        quint64 idleDelta = idle - m_previousIdle;
        loadPercent = 100.0 * static_cast<double>(totalDelta - idleDelta)
                      / static_cast<double>(totalDelta);
    }

    m_previousTotal = total;
    m_previousIdle = idle;
    m_hasPrevious = true;

    return qBound(0.0, loadPercent, 100.0);
}