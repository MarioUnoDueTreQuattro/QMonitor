#ifndef NETGRAPHWIDGET_H
#define NETGRAPHWIDGET_H

#include <QWidget>
#include <QVector>
#include <QDateTime>
#include <QPen>

class NetGraphWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NetGraphWidget(QWidget *parent = nullptr);
    void addData(qint64 rxSpeed, qint64 txSpeed);
    void resetGraph();
    void setPaused(bool);
    void setDownloadColor(const QColor &newColor);
    void setUploadColor(const QColor &newColor);
    void setDrawMode(int newMode);
    QPixmap renderTrayIcon(int size);
    qint64 getMaxRx() const;
    qint64 getMaxTx() const;
    qint64 getAvgRx() const;
    qint64 getAvgTx() const;
    bool paused() const;
    enum DrawMode { DrawModeLines, DrawModeBars };
    void setUpdateIntervalNet(int newUpdateIntervalNet);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
private slots:
    void showContextMenu(const QPoint &pos);
private:
    DrawMode m_drawMode = DrawModeLines;
    void drawSeriesBars(QPainter &painter, const QPen &pen, bool useRx,
        const QRect &graphRect, bool isTray);
    void drawSeriesLines(QPainter &painter, const QPen &pen, bool useRx, const QRect &graphRect, double stepX);
    struct DataPoint
    {
        qint64 rx;
        qint64 tx;
        qint64 timestamp;
    };
    QVector<DataPoint> m_history;
    int m_maxHistorySize; // Numero di campioni visibili (es. 60 secondi)
    qint64 m_maxVal;      // Valore massimo corrente sull'asse Y (in Byte/s)
    bool m_paused = false;
    QString formatBytes(qint64 bytes) const;
    QFont m_gridFont;
    QPen m_penRx{QColor(0, 211, 10), 1.5};
    QPen m_penTx{QColor(255, 57, 0), 1.5};
    QPen m_gridPen{QColor(48, 48, 48), 1, Qt::DashLine};
    QPen m_borderPen{QColor(48, 48, 48)};
    QPen m_labelPen{QColor(160, 160, 160)};
    qint64 getTrayVisibleMax(int visibleSamples) const;
    int m_updateIntervalNet;
signals:
    void settingsRequested(); // custom signal
    void pauseStateChanged(); // custom signal

};

#endif // NETGRAPHWIDGET_H