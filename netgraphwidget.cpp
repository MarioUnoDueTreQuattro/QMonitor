#include "netgraphwidget.h"
#include "utils.h"
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
    setMinimumSize(80, 60);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, &NetGraphWidget::showContextMenu);
    m_gridFont = QFont("SansSerif", 8);
    QFontMetrics fm(m_gridFont);
    m_labelHeight = fm.height();
    m_labelHeight = m_labelHeight + 1; // vertical breathing room between rows
    m_updateIntervalNet = 2000;
}

void NetGraphWidget::showContextMenu(const QPoint &pos)
{
    QMenu menu(this);
    QAction *actHide = menu.addAction(tr("Hide"));
    actHide->setIcon(QIcon::fromTheme("password-show-off"));
    QAction *actReset = menu.addAction(tr("Reset"));
    actReset->setIcon(QIcon::fromTheme("system-reboot"));
    QAction *actPause = menu.addAction(tr("Pause"));
    actPause->setIcon(QIcon::fromTheme("media-playback-pause"));
    actPause->setCheckable(true);
    actPause->setChecked(m_paused);
    menu.addSeparator();
    QAction *actSettings = menu.addAction(tr("Settings..."));
    actSettings->setIcon(QIcon::fromTheme("configure"));
    actSettings->setIcon(QIcon::fromTheme("configure"));
    QAction *actAbout = menu.addAction(tr("About..."));
    actAbout->setIcon(QIcon::fromTheme("QMonitor"));
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
    else if (selected == actAbout)
    {
        emit aboutRequested();
    }
    else if (selected == actHide)
    {
        emit hideRequested(this->parentWidget());
    }
}

bool NetGraphWidget::paused() const
{
    return m_paused;
}

void NetGraphWidget::addData(qint64 rxSpeed, qint64 txSpeed)
{
    //qint64 now = QDateTime::currentMSecsSinceEpoch() / m_updateIntervalNet;
    m_history.append({rxSpeed, txSpeed});
    // Calcoliamo lo spazio disponibile per adattare lo storico ai pixel correnti
    int marginLeft = 55;
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
    //m_maxVal = static_cast<qint64>(m_maxVal * 1.05);
    update();
}

void NetGraphWidget::resetGraph()
{
    m_sampleHistory->clear();
    m_sampleHistory->setMinimumLevel(1024);
    m_history.clear();
    m_maxVal = 1024; // Riporta la soglia al minimo iniziale (1 KB/s)
    update();
}

void NetGraphWidget::setPaused(bool pausedState)
{
    m_paused = pausedState;
    emit pauseStateChanged();
}

// QString NetGraphWidget::formatBytes(qint64 bytes) const
// {
// if (bytes < 1024) return QString("%1 B").arg(bytes);
// const bool isKb = bytes < BYTES_PER_MB;
// const double value = isKb ? bytes / 1024.0 : bytes / BYTES_PER_MB;
// const char *unit = isKb ? "KB" : "MB";
// if (value < 20.0) return QString("%1 %2").arg(value, 0, 'f', 1).arg(unit);
// return QString("%1 %2").arg(qRound(value)).arg(unit);
// }

/*
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
*/

void NetGraphWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    // Quando la finestra viene ridimensionata, tronchiamo subito lo storico
    // se eccede la nuova larghezza disponibile
    int marginLeft = 55;
    int marginRight = 0;
    int graphW = width() - marginLeft - marginRight;
    if (graphW > 10 && m_history.size() > graphW)
    {
        m_history.remove(0, m_history.size() - graphW);
    }
    if (m_sampleHistory != nullptr)
    {
        // qDebug() << "graphW: " << graphW;
        // if (graphW > 10 && m_sampleHistory->sampleCount() > graphW)
        // {
        // m_sampleHistory->setMaxSamples(graphW);
        // }
        m_sampleHistory->setMaxSamples(graphW);
    }
}

void NetGraphWidget::drawSeriesLines(QPainter &painter, const QPen &pen, bool useRx,
    const QRect &graphRect, double stepX)
{
    if (m_maxVal <= 0)
        return;   // avoid division by zero
    //const double bottom = graphRect.top() + graphRect.height();
    const double bottom = graphRect.bottom(); // NON top + height
    const double top = graphRect.top();
    QPainterPath path;
    for (int i = 0; i < m_history.size(); ++i)
    {
        double x = graphRect.left() + (i * stepX);
        qint64 val = useRx ? m_history[i].rx : m_history[i].tx;
        double y = bottom - (static_cast<double>(val) / m_maxVal) * graphRect.height();
        // Keep the line inside the graph area
        // y = qBound(static_cast<double>(graphRect.top()), y, bottom);
        y = qBound(top, y, bottom); // ora il bound inferiore è corretto
        if (i == 0)
            path.moveTo(x, y);
        else
            path.lineTo(x, y);
    }
    /*
        painter.save();
        painter.setClipRect(graphRect);   // <-- taglia lo sbordamento della penna 1.5
        painter.setPen(pen);
        painter.drawPath(path);
        painter.restore();

        */
    painter.setPen(pen);
    painter.drawPath(path);
}

void NetGraphWidget::setMaxVal(qint64 newMaxVal)
{
    m_maxVal = newMaxVal;
}

void NetGraphWidget::setColors(const QVector<QColor> &newColors)
{
    m_colors = newColors;
}

void NetGraphWidget::setBorderColor(const QColor &newColor)
{
    m_borderPen.setColor(newColor);
}

void NetGraphWidget::setGridLinesColor(const QColor &newColor)
{
    m_gridPen.setColor(newColor);
}

void NetGraphWidget::setGridLinesStyle(int rawValue)
{
    m_gridPen.setStyle(static_cast<Qt::PenStyle>(rawValue));
}

void NetGraphWidget::setSampleHistory(SampleHistory *history)
{
    m_sampleHistory = history;
}

void NetGraphWidget::drawSeriesBars(QPainter &painter, const QPen &pen, bool useRx, const QRect &graphRect, bool isTray)
{
    if (m_maxVal <= 0 || m_history.isEmpty())
        return;
    const int bottom = graphRect.bottom();
    const int samplesToDraw = qMin(static_cast<int>(m_history.size()), graphRect.width());
    const int firstSample = m_history.size() - samplesToDraw;
    // The newest sample is on the right edge, the oldest scrolls out on the left
    const int firstX = graphRect.right() - samplesToDraw + 1;
    QVector<QLine> lines;
    lines.reserve(samplesToDraw);
    qint64 maxTrayVal;
    if (isTray) maxTrayVal = getTrayVisibleMax(samplesToDraw);
    else maxTrayVal = m_maxVal;
    for (int i = 0; i < samplesToDraw; ++i)
    {
        qint64 val = useRx ? m_history[firstSample + i].rx : m_history[firstSample + i].tx;
        int barHeight = static_cast<int>(static_cast<double>(val) / maxTrayVal * graphRect.height());
        barHeight = qBound(0, barHeight, graphRect.height());
        if (barHeight == 0) continue; // niente linea sul bordo per valori ~0
        int x = firstX + i;
        lines.append(QLine(x, bottom, x, bottom - barHeight));
    }
    // Integer coordinates + 1 px pen: no antialiasing needed, and it is much cheaper
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(pen);
    painter.drawLines(lines);
}

void NetGraphWidget::drawSeriesBars2(QPainter &painter, const QRect &graphRect,
    const SampleHistory &history,
    const QVector<QColor> &seriesColors, double maxValue)
{
    if (m_maxVal <= 0 || history.sampleCount() == 0)
        return;
    // qDebug() << "history.sampleCount():" << history.sampleCount();
    const int bottom = graphRect.bottom();
    const int samplesToDraw = qMin(history.sampleCount(), graphRect.width());
    // qDebug() << "samplesToDraw:" << samplesToDraw;
    const int firstSample = history.sampleCount() - samplesToDraw;
    const int firstX = graphRect.right() - samplesToDraw + 1;
    painter.setRenderHint(QPainter::Antialiasing, false);
    for (int s = 0; s < history.seriesCount() && s < seriesColors.size(); ++s)
    {
        QVector<QLine> lines;
        lines.reserve(samplesToDraw);
        for (int i = 0; i < samplesToDraw; ++i)
        {
            double value = history.valueAt(s, firstSample + i);
            int barHeight = static_cast<int>(value / maxValue * graphRect.height());
            barHeight = qBound(0, barHeight, graphRect.height());
            if (barHeight == 0) continue; //avoid bottom line
            int x = firstX + i;
            lines.append(QLine(x, bottom, x, bottom - barHeight + 1));
        }
        painter.setPen(QPen(seriesColors.at(s), 1));
        painter.drawLines(lines);
    }
}

qint64 NetGraphWidget::getTrayVisibleMax(int visibleSamples) const
{
    qint64 maxVal = 1;
    int first =
        qMax(0,
            m_history.size() - visibleSamples);
    for (int i = first; i < m_history.size(); ++i)
    {
        maxVal = qMax(maxVal, m_history[i].rx);
        maxVal = qMax(maxVal, m_history[i].tx);
    }
    return maxVal;
}

void NetGraphWidget::setMaxGridDivisions(int newMaxGridDivisions)
{
    m_maxGridDivisions = newMaxGridDivisions;
}

QPixmap NetGraphWidget::renderTrayIcon(int size)
{
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);   // or a dark background, see below
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, false);
    if (m_history.size() >= 1)
    {
        QRect graphRect(0, 0, size, size);
        drawSeriesBars(painter, m_penRx, true, graphRect, true);
        drawSeriesBars(painter, m_penTx, false, graphRect, true);
    }
    return pixmap;
}

QPixmap NetGraphWidget::renderTrayPixmap(SampleHistory &history,
    QVector<QColor> &colors, double maxValue, int size)
{
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    paintTrayBars(painter, QRect(0, 0, size, size), history, colors, maxValue);
    return pixmap;
}

void NetGraphWidget::paintTrayBars(QPainter &painter, const QRect &graphRect,
    const SampleHistory &history,
    const QVector<QColor> &seriesColors, double maxValue)
{
    if (maxValue <= 0.0 || history.sampleCount() == 0)
        return;
    const int bottom = graphRect.bottom();
    const int samplesToDraw = qMin(history.sampleCount(), graphRect.width());
    const int firstSample = history.sampleCount() - samplesToDraw;
    const int firstX = graphRect.right() - samplesToDraw + 1;
    painter.setRenderHint(QPainter::Antialiasing, false);
    for (int s = 0; s < history.seriesCount() && s < seriesColors.size(); ++s)
    {
        QVector<QLine> lines;
        lines.reserve(samplesToDraw);
        for (int i = 0; i < samplesToDraw; ++i)
        {
            double value = history.valueAt(s, firstSample + i);
            int barHeight = static_cast<int>(value / maxValue * graphRect.height());
            barHeight = qBound(0, barHeight, graphRect.height());
            if (barHeight == 0) continue; //avoid bottom line
            int x = firstX + i;
            lines.append(QLine(x, bottom, x, bottom - barHeight + 1));
        }
        painter.setPen(QPen(seriesColors.at(s), 1));
        painter.drawLines(lines);
    }
}

void NetGraphWidget::setUpdateIntervalNet(int newUpdateIntervalNet)
{
    m_updateIntervalNet = newUpdateIntervalNet;
}

void NetGraphWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);
    int w = width();
    int h = height();
    int marginLeft = 55;
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
    painter.setPen(m_borderPen);
    painter.drawRect(graphX, graphY, graphW - 1, graphH - 1);
    //painter.setPen(m_gridPen);
    painter.setFont(m_gridFont);
    int hDivisions = graphH / m_labelHeight;
    hDivisions = qMin(hDivisions, m_maxGridDivisions);
    if (m_useHumanDivisions && m_unitMode == UnitMode::UnitModePercent)
    {
        int best = 1;
        for (int d : kHumanDivisors100)
        {
            if (d > hDivisions)
                break;
            best = d;
        }
        hDivisions = best;
    }
    QString label;
    painter.setPen(m_labelPen);
    for (int i = 0; i < hDivisions; ++i)
    {
        const int y = graphY + (graphH * i / hDivisions);
        //if (i != 0) painter.drawLine(graphX, y, graphX + graphW, y);
        qint64 val = m_maxVal - (m_maxVal * i / hDivisions);
        switch (m_unitMode)
        {
            case UnitMode::UnitModeNet:
                label = Utils::formatBytes(val) /*+ "/s"*/;
                break;
            case UnitMode::UnitModePercent:
                {
                    const qint64 val10 = (m_maxVal * 10) - (m_maxVal * 10 * i / hDivisions);
                    label = QString::number(val10 / 10.0) + "%"; // es. 875 / 10.0 -> "87.5%"
                    // label = QString::number(val) + "%";
                    break;
                }
            case UnitMode::UnitModeMemory:
                label = QString::number(int(val / BYTES_PER_MB)) + " MB";
                break;
                // default:
                // break;
        }
        // if (m_unitMode == static_cast<int>(UnitMode::UnitModeNet)) label = Utils::formatBytes(val) /*+ "/s"*/;
        // painter.setPen(m_labelPen);
        /*if (i != hDivisions) */painter.drawText(QRect(5, y - 8, marginLeft - 10, 16), Qt::AlignRight | Qt::AlignVCenter, label);
        //painter.setPen(m_gridPen);
    }
    // int vDivisions = 6;
    // for (int i = 0; i <= vDivisions; ++i)
    // {
    // int x = graphX + (graphW * i / vDivisions);
    // painter.drawLine(x, graphY, x, graphY + graphH);
    // }
    bool bDrawGraph = true;
    if (m_history.size() < 2 && m_sampleHistory->sampleCount() < 2) bDrawGraph = false;
    // Ora stepX è esattamente (o vicinissimo a) 1.0 pixel per ogni campione!
    //double stepX = static_cast<double>(graphW) / (m_history.size() - 1);
    if (bDrawGraph)
    {
        QRect graphRect(graphX, graphY, graphW, graphH);
        if (m_drawMode == DrawMode::DrawModeLines) painter.setRenderHint(QPainter::Antialiasing);
        else painter.setRenderHint(QPainter::Antialiasing, false);
        // if (m_drawMode == DrawMode::DrawModeBars)
        if (true)
        {
            // drawSeriesBars(painter, m_penRx, true, graphRect, false);
            // drawSeriesBars(painter, m_penTx, false, graphRect, false);
            drawSeriesBars2(painter, graphRect, *m_sampleHistory, m_colors, m_maxVal);
        }
        else
        {
            double stepX = static_cast<double>(graphRect.width() - 1) / (m_history.size() - 1);
            drawSeriesLines(painter, m_penRx, true, graphRect, stepX);
            drawSeriesLines(painter, m_penTx, false, graphRect, stepX);
        }
    }
    // Fase 1: linee della griglia orizzontale
    painter.setPen(m_gridPen);
    for (int i = 1; i < hDivisions; ++i)   // parte da 1: la linea i=0 non si disegna
    {
        const int y = graphY + (graphH * i / hDivisions);
        painter.drawLine(graphX, y, graphX + graphW, y);
    }
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

void NetGraphWidget::setDrawMode(DrawMode newMode)
{
    // if (newMode == m_drawMode)
    // return;
    // m_drawMode = static_cast<DrawMode>(newMode);
    m_drawMode = newMode;
    if (m_drawMode == DrawMode::DrawModeLines)
    {
        m_penRx.setWidth(1.5);
        m_penTx.setWidth(1.5);
    }
    else
    {
        m_penRx.setWidth(1);
        m_penTx.setWidth(1);
    }
    update();
}

void NetGraphWidget::setUnitMode(UnitMode newMode)
{
    m_unitMode = newMode;
    if (m_unitMode == NetGraphWidget::UnitMode::UnitModePercent) m_maxVal = 100;
    // else m_maxVal = m_sampleHistory->maxValueInLast(m_sampleHistory->sampleCount(), 0);
    update();
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
