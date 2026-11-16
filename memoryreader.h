#ifndef MEMORYREADER_H
#define MEMORYREADER_H

#include <QByteArray>

struct MemorySnapshot
{
    double ramUsedBytes;
    double swapUsedBytes;   // swap on disk only (zram excluded)
    double zramUsedBytes;       // uncompressed or physical RAM, see read()
    double maxTotalBytes;       // highest among ram, disk swap and zram totals
};

class MemoryReader
{
public:
    // zramPhysical = false: uncompressed data stored (default)
    // zramPhysical = true:  physical RAM occupied by zram after compression
    static MemorySnapshot read(bool zramPhysical);

private:
    static double readMeminfoKilobytes(const QByteArray &allContent, const QByteArray &key);
    static double readZramBytes(const QString &deviceName, bool zramPhysical);
};

#endif // MEMORYREADER_H
