#include "netgraphwidget.h"
#include <QPainter>
#include <QPen>
#include <QColor>
#include <QPainterPath>
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>

NetGraphWidget::NetGraphWidget(QWidget *parent)
    : QWidget(parent)
    , m_maxVal(1024)
{
    setMinimumSize(250, 100);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested,
        this, &NetGraphWidget::showContextMenu);
    m_gridFont = QFont("SansSerif", 8);
}

void NetGraphWidget::showContextMenu(const QPoint &pos)
{
    QMenu menu(this);
    QAction *actReset = menu.addAction(tr("Reset"));
    QAction *actPause = menu.addAction(tr("Pause"));
    actPause->setCheckable(true);
    actPause->setChecked(m_paused);
    menu.addSeparator();
    QAction *actSettings = menu.addAction(tr("Settings..."));
    // Esecuzione modale: restituisce l'action scelta (o nullptr)
    QAction *selected = menu.exec(mapToGlobal(pos));
    if (!selected)
        return;
    if (selected == actReset)
    {
        resetGraph();
    }
    else if (selected == actPause)
    {
        setPaused(selected->isChecked());
    }
    else if (selected == actSettings)
    {
        emit settingsRequested();
    }
}

bool NetGraphWidget::paused() const
{
    return m_paused;
}

void NetGraphWidget::addData(qint64 rxSpeed, qint64 txSpeed)
{
    qint64 now = QDateTime::currentMSecsSinceEpoch() / 2000;
    m_history.append({rxSpeed, txSpeed, now});
    // Calcoliamo lo spazio disponibile per adattare lo storico ai pixel correnti
    int marginLeft = 60;
    int marginRight = 0;
    int graphW = width() - marginLeft - marginRight;
    // Se la larghezza è valida, tronchiamo lo storico in eccesso per mantenere 1 punto = 1 pixel
    if (graphW > 10)
    {
        while (m_history.size() > graphW)
        {
            m_history.removeFirst();
        }
    }
    // Ricalcolo del valore massimo dinamico
    m_maxVal = 1024;
    for (const auto &pt : m_history)
    {
        if (pt.rx > m_maxVal) m_maxVal = pt.rx;
        if (pt.tx > m_maxVal) m_maxVal = pt.tx;
    }
    m_maxVal = static_cast<qint64>(m_maxVal * 1.1);
    update();
}

void NetGraphWidget::resetGraph()
{
    m_history.clear();
    m_maxVal = 1024; // Riporta la soglia al minimo iniziale (1 KB/s)
    update();
}

void NetGraphWidget::setPaused(bool paused)
{
    m_paused = paused;
}

QString NetGraphWidget::formatBytes(qint64 bytes) const
{
    if (bytes < 1024)
        return QString("%1 B").arg(bytes);
    if (bytes < 1024LL * 1024)
        return QString("%1 KB").arg(bytes / 1024);
    if (bytes < 1024LL * 1024 * 1024)
        return QString("%1 MB").arg(bytes / (1024.0 * 1024.0), 0, 'f', 1);
    return QString("%1 GB").arg(bytes / (1024.0 * 1024.0 * 1024.0), 0, 'f', 1);
}

void NetGraphWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    // Quando la finestra viene ridimensionata, tronchiamo subito lo storico
    // se eccede la nuova larghezza disponibile
    int marginLeft = 60;
    int marginRight = 0;
    int graphW = width() - marginLeft - marginRight;
    if (graphW > 10 && m_history.size() > graphW)
    {
        m_history.remove(0, m_history.size() - graphW);
    }
}

void NetGraphWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    int w = width();
    int h = height();
    int marginLeft = 60;
    int marginRight = 0;
    int marginTop = 5;
    int marginBottom = 0;
    int graphX = marginLeft;
    int graphY = marginTop;
    int graphW = w - marginLeft - marginRight;
    int graphH = h - marginTop - marginBottom;
    if (graphW <= 50 || graphH <= 30) return;
    // Controllo di sicurezza sincronizzato per il ridimensionamento della finestra
    // if (m_history.size() > graphW)
    // {
    // m_history.remove(0, m_history.size() - graphW);
    // }
    // Sfondo generale e area grafico
    //painter.fillRect(rect(), QColor(25, 25, 25));
    //painter.fillRect(graphX, graphY, graphW, graphH, QColor(30, 30, 30));
    // --- GRIGLIA TRATTEGGIATA ---
    //QPen gridPen(QColor(55, 55, 55), 1, Qt::DashLine);
    painter.setPen(m_gridPen);
    painter.setFont(m_gridFont);
    int hDivisions = 4;
    for (int i = 0; i <= hDivisions; ++i)
    {
        int y = graphY + (graphH * i / hDivisions);
        painter.drawLine(graphX, y, graphX + graphW, y);
        qint64 val = m_maxVal - (m_maxVal * i / hDivisions);
        QString label = formatBytes(val) + "/s";
        painter.setPen(m_labelPen);
        if (i != hDivisions) painter.drawText(QRect(5, y - 8, marginLeft - 10, 16), Qt::AlignRight | Qt::AlignVCenter, label);
        painter.setPen(m_gridPen);
    }
    // int vDivisions = 6;
    // for (int i = 0; i <= vDivisions; ++i)
    // {
    // int x = graphX + (graphW * i / vDivisions);
    // painter.drawLine(x, graphY, x, graphY + graphH);
    // }
    painter.setPen(m_borderPen);
    painter.drawRect(graphX, graphY, graphW, graphH);
    if (m_history.size() < 2) return;
    // Ora stepX è esattamente (o vicinissimo a) 1.0 pixel per ogni campione!
    double stepX = static_cast<double>(graphW) / (m_history.size() - 1);
    auto drawSeries = [&](auto extractValue, const QPen & pen)
    {
        painter.setPen(pen);
        QPainterPath path;
        bool first = true;
        for (int i = 0; i < m_history.size(); ++i)
        {
            double x = graphX + (i * stepX);
            qint64 val = extractValue(m_history[i]);
            double y = (graphY + graphH) - (static_cast<double>(val) / m_maxVal) * graphH;
            if (y < graphY) y = graphY;
            if (y > graphY + graphH) y = graphY + graphH;
            if (first)
            {
                path.moveTo(x, y);
                first = false;
            }
            else
            {
                path.lineTo(x, y);
            }
        }
        painter.drawPath(path);
    };
    drawSeries([](const DataPoint & p) { return p.rx; }, m_penRx);
    drawSeries([](const DataPoint & p) { return p.tx; }, m_penTx);
}

qint64 NetGraphWidget::getMaxRx() const
{
    qint64 maxVal = 0;
    for (const auto &pt : m_history)
    {
        if (pt.rx > maxVal) maxVal = pt.rx;
    }
    return maxVal;
}

qint64 NetGraphWidget::getMaxTx() const
{
    qint64 maxVal = 0;
    for (const auto &pt : m_history)
    {
        if (pt.tx > maxVal) maxVal = pt.tx;
    }
    return maxVal;
}

qint64 NetGraphWidget::getAvgRx() const
{
    if (m_history.isEmpty()) return 0;
    qint64 sum = 0;
    for (const auto &pt : m_history)
    {
        sum += pt.rx;
    }
    return sum / m_history.size();
}

qint64 NetGraphWidget::getAvgTx() const
{
    if (m_history.isEmpty()) return 0;
    qint64 sum = 0;
    for (const auto &pt : m_history)
    {
        sum += pt.tx;
    }
    return sum / m_history.size();
}

void NetGraphWidget::setDownloadColor(const QColor &newColor)
{
    if (!newColor.isValid() || newColor == m_penRx.color())
        return;
    m_penRx.setColor(newColor);
    update();   // schedule a repaint
}

void NetGraphWidget::setUploadColor(const QColor &newColor)
{
    if (!newColor.isValid() || newColor == m_penTx.color())
        return;
    m_penTx.setColor(newColor);
    update();   // schedule a repaint
}
