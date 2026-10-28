#include "netgraphwidget.h"
#include <QPainter>
#include <QPen>
#include <QColor>

NetGraphWidget::NetGraphWidget(QWidget *parent)
    : QWidget(parent), m_maxVal(1024)
{
    setFixedHeight(60);
}

void NetGraphWidget::addData(qint64 rxSpeed, qint64 txSpeed)
{
    m_rxHistory.append(rxSpeed);
    m_txHistory.append(txSpeed);

    // Mantiene una finestra fissa di 55 campioni
    if (m_rxHistory.size() > 55) m_rxHistory.removeFirst();
    if (m_txHistory.size() > 55) m_txHistory.removeFirst();

    // Calcolo della scala dinamica (minimo 1 KB/s)
    m_maxVal = 1024;
    for (qint64 val : m_rxHistory) if (val > m_maxVal) m_maxVal = val;
    for (qint64 val : m_txHistory) if (val > m_maxVal) m_maxVal = val;

    // Forza il ridisegno del widget
    update();
}

void NetGraphWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Sfondo scuro pulito (stile terminale/monitor)
    painter.fillRect(rect(), QColor(30, 30, 30));

    int w = width();
    int h = height();
    int n = m_rxHistory.size();
    if (n < 2) return;

    double step = static_cast<double>(w) / 55.0;

    // Disegno Download (Verde)
    painter.setPen(QPen(Qt::green, 1.5));
    for (int i = 1; i < n; ++i) {
        double x1 = (i - 1) * step;
        double y1 = h - (static_cast<double>(m_rxHistory[i-1]) / m_maxVal) * (h - 6);
        double x2 = i * step;
        double y2 = h - (static_cast<double>(m_rxHistory[i]) / m_maxVal) * (h - 6);
        painter.drawLine(QPointF(x1, y1), QPointF(x2, y2));
    }

    // Disegno Upload (Rosso)
    painter.setPen(QPen(Qt::red, 1.5));
    for (int i = 1; i < n; ++i) {
        double x1 = (i - 1) * step;
        double y1 = h - (static_cast<double>(m_txHistory[i-1]) / m_maxVal) * (h - 6);
        double x2 = i * step;
        double y2 = h - (static_cast<double>(m_txHistory[i]) / m_maxVal) * (h - 6);
        painter.drawLine(QPointF(x1, y1), QPointF(x2, y2));
    }
}