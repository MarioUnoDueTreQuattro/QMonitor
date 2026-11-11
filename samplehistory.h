#ifndef SAMPLEHISTORY_H
#define SAMPLEHISTORY_H

#include <QObject>
#include <QVector>

// Generic history of samples with a fixed number of series.
// Network: 2 series (rx, tx) - CPU: 1 series - Memory: 3 series (ram, swap, zram).
// No graphic dependencies: widgets and tray icons only read from here.
class SampleHistory : public QObject
{
    Q_OBJECT

public:
    explicit SampleHistory(int seriesCount, QObject *parent = nullptr);

    // 'values' must contain exactly seriesCount() values, in series order
    void addSample(const QVector<double> &values);
    void clear();
    void setMaxSamples(int newMaxSamples);

    int seriesCount() const;
    int sampleCount() const;
    double valueAt(int seriesIndex, int sampleIndex) const;   // sampleIndex 0 = oldest

    // Highest value among all series in the newest 'visibleSamples' samples,
    // never lower than 'minimumValue' (avoids zooming in on noise)
    double maxValueInLast(int visibleSamples, double minimumValue) const;

signals:
    void historyChanged();   // emitted once per new sample, and after clear()

private:
    void trimToMaxSamples();

    // One vector per series: drawing a series reads a single contiguous vector
    QVector< QVector<double> > m_series;
    int m_maxSamples;
};

#endif // SAMPLEHISTORY_H