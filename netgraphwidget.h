#ifndef NETGRAPHWIDGET_H
#define NETGRAPHWIDGET_H

#include <QWidget>
#include <QVector>
#include <QPaintEvent>

class NetGraphWidget : public QWidget {
    Q_OBJECT
public:
    explicit NetGraphWidget(QWidget *parent = nullptr);
    void addData(qint64 rxSpeed, qint64 txSpeed);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<qint64> m_rxHistory;
    QVector<qint64> m_txHistory;
    qint64 m_maxVal;
};

#endif // NETGRAPHWIDGET_H