#ifndef NETGRAPHWIDGET_H
#define NETGRAPHWIDGET_H

#include <QWidget>
#include <QVector>
#include <QDateTime>

class NetGraphWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NetGraphWidget(QWidget *parent = nullptr);
    void addData(qint64 rxSpeed, qint64 txSpeed);
    void resetGraph();
    qint64 getMaxRx() const;
    qint64 getMaxTx() const;
    qint64 getAvgRx() const;
    qint64 getAvgTx() const;
protected:
    void paintEvent(QPaintEvent *event) override;

private:
    struct DataPoint
    {
        qint64 rx;
        qint64 tx;
        qint64 timestamp;
    };

    QVector<DataPoint> m_history;
    int m_maxHistorySize; // Numero di campioni visibili (es. 60 secondi)
    qint64 m_maxVal;      // Valore massimo corrente sull'asse Y (in Byte/s)

    QString formatBytes(qint64 bytes) const;
};

#endif // NETGRAPHWIDGET_H