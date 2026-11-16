#include "memoryreader.h"
#include <QFile>

// memoryreader.cpp (new or changed parts)
double MemoryReader::readZramBytes(const QString &deviceName, bool zramPhysical)
{
    // mm_stat fields: 0 = orig_data_size, 1 = compr_data_size, 2 = mem_used_total
    QFile mmStat("/sys/block/" + deviceName + "/mm_stat");
    if (!mmStat.open(QIODevice::ReadOnly))
        return 0.0;

    QStringList fields = QString::fromLatin1(mmStat.readAll()).simplified().split(' ');
    int fieldIndex = zramPhysical ? 2 : 0;
    if (fields.size() <= fieldIndex)
        return 0.0;
    return fields.at(fieldIndex).toDouble();
}

double MemoryReader::readMeminfoKilobytes(const QByteArray &allContent, const QByteArray &key)
{
    // Lines look like "MemTotal:       16318476 kB"
    QList<QByteArray> lines = allContent.split('\n');
    for (int i = 0; i < lines.size(); ++i)
    {
        if (lines.at(i).startsWith(key + ":"))
        {
            QStringList parts = QString::fromLatin1(lines.at(i)).simplified().split(' ');
            if (parts.size() >= 2)
                return parts.at(1).toDouble();
        }
    }
    return 0.0;
}

MemorySnapshot MemoryReader::read(bool zramPhysical)
{
    MemorySnapshot snapshot = { 0.0, 0.0, 0.0, 0.0 };

    QFile meminfo("/proc/meminfo");
    if (!meminfo.open(QIODevice::ReadOnly))
        return snapshot;
    QByteArray content = meminfo.readAll();
    meminfo.close();

    double ramTotal = readMeminfoKilobytes(content, "MemTotal") * 1024.0;
    double ramAvailable = readMeminfoKilobytes(content, "MemAvailable") * 1024.0;
    snapshot.ramUsedBytes = ramTotal - ramAvailable;

    // /proc/swaps columns: Filename Type Size Used Priority (sizes in KiB)
    double diskSwapTotal = 0.0;
    double zramTotal = 0.0;

    QFile swaps("/proc/swaps");
    if (swaps.open(QIODevice::ReadOnly))
    {
        QStringList lines = QString::fromLatin1(swaps.readAll()).split('\n');
        for (int i = 1; i < lines.size(); ++i)   // line 0 is the header
        {
            QStringList columns = lines.at(i).simplified().split(' ');
            if (columns.size() < 4)
                continue;

            QString fileName = columns.at(0);
            double sizeBytes = columns.at(2).toDouble() * 1024.0;
            double usedBytes = columns.at(3).toDouble() * 1024.0;

            if (fileName.startsWith("/dev/zram"))
            {
                zramTotal += sizeBytes;
                snapshot.zramUsedBytes += readZramBytes(fileName.mid(5), zramPhysical);   // "zram0"
            }
            else
            {
                diskSwapTotal += sizeBytes;
                snapshot.swapUsedBytes += usedBytes;
            }
        }
    }

    snapshot.maxTotalBytes = qMax(ramTotal, qMax(diskSwapTotal, zramTotal));
    return snapshot;
}
