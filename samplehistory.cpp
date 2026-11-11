#include "samplehistory.h"

SampleHistory::SampleHistory(int seriesCount, QObject *parent)
    : QObject(parent),
      m_series(seriesCount),
      m_maxSamples(4096)
{
}

void SampleHistory::addSample(const QVector<double> &values)
{
    if (values.size() != m_series.size())
        return;   // wrong number of values: ignore instead of corrupting the history

    for (int i = 0; i < m_series.size(); ++i)
        m_series[i].append(values.at(i));

    trimToMaxSamples();
    emit historyChanged();
}

void SampleHistory::clear()
{
    for (int i = 0; i < m_series.size(); ++i)
        m_series[i].clear();

    emit historyChanged();
}

void SampleHistory::setMaxSamples(int newMaxSamples)
{
    if (newMaxSamples < 1)
        return;

    m_maxSamples = newMaxSamples;
    trimToMaxSamples();
}

void SampleHistory::trimToMaxSamples()
{
    // All series always have the same length
    for (int i = 0; i < m_series.size(); ++i)
    {
        int excess = m_series.at(i).size() - m_maxSamples;
        if (excess > 0)
            m_series[i].remove(0, excess);
    }
}

int SampleHistory::seriesCount() const
{
    return m_series.size();
}

int SampleHistory::sampleCount() const
{
    return m_series.isEmpty() ? 0 : m_series.at(0).size();
}

double SampleHistory::valueAt(int seriesIndex, int sampleIndex) const
{
    return m_series.at(seriesIndex).at(sampleIndex);
}

double SampleHistory::maxValueInLast(int visibleSamples, double minimumValue) const
{
    double maxValue = minimumValue;
    int firstIndex = qMax(0, sampleCount() - visibleSamples);

    for (int s = 0; s < m_series.size(); ++s)
    {
        for (int i = firstIndex; i < m_series.at(s).size(); ++i)
            maxValue = qMax(maxValue, m_series.at(s).at(i));
    }
    return maxValue;
}