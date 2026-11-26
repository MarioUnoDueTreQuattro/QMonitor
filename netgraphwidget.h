#ifndef NETGRAPHWIDGET_H
#define NETGRAPHWIDGET_H

#include "samplehistory.h"
#include <QWidget>
#include <QVector>
#include <QDateTime>
#include <QPen>

class NetGraphWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NetGraphWidget(QWidget *parent = nullptr);
    enum class DrawMode { DrawModeLines, DrawModeBars };
    enum class UnitMode { UnitModeNet, UnitModePercent, UnitModeMemory };
    void addData(qint64 rxSpeed, qint64 txSpeed);
    void resetGraph();
    void setPaused(bool);
    void setDownloadColor(const QColor &newColor);
    void setUploadColor(const QColor &newColor);
    void setDrawMode(DrawMode newMode);
    void setUnitMode(UnitMode newMode);
    QPixmap renderTrayIcon(int size);
    static QPixmap renderTrayPixmap(SampleHistory &history, QVector<QColor> &colors, double maxValue, int size);
    qint64 getMaxRx() const;
    qint64 getMaxTx() const;
    qint64 getAvgRx() const;
    qint64 getAvgTx() const;
    bool paused() const;
    void setUpdateIntervalNet(int newUpdateIntervalNet);
    void setSampleHistory(SampleHistory *newSampleHistory);
    void setColors(const QVector<QColor> &newColors);
    void setBorderColor(const QColor &newColor);
    void setGridLinesColor(const QColor &newColor);
    void setGridLinesStyle(int);
    void setLabelColor(const QColor &newColor);
    void setMaxVal(qint64 newMaxVal);
    void setMaxGridDivisions(int newMaxGridDivisions);

    void updateToolTipLive(QWidget *widget, const QString &newText);
protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
private slots:
    void showContextMenu(const QPoint &pos);
private:
    DrawMode m_drawMode = DrawMode::DrawModeLines;
    UnitMode m_unitMode = UnitMode::UnitModeNet;
    void drawSeriesBars(QPainter &painter, const QPen &pen, bool useRx, const QRect &graphRect, bool isTray);
    void drawSeriesBars2(QPainter &painter, const QRect &graphRect,
        const SampleHistory &history,
        const QVector<QColor> &seriesColors, double maxValue);
    static void paintTrayBars(QPainter &painter, const QRect &graphRect, const SampleHistory &history, const QVector<QColor> &seriesColors, double maxValue);
    void drawSeriesLines(QPainter &painter, const QPen &pen, bool useRx, const QRect &graphRect, double stepX);
    struct DataPoint
    {
        qint64 rx;
        qint64 tx;
        //qint64 timestamp;
    };
    SampleHistory *m_sampleHistory;
    QVector<QColor> m_colors;
    QVector<DataPoint> m_history;
    int m_maxHistorySize; // Numero di campioni visibili (es. 60 secondi)
    qint64 m_maxVal;      // Valore massimo corrente sull'asse Y (in Byte/s)
    bool m_paused = false;
    // QString formatBytes(qint64 bytes) const;
    QFont m_gridFont;
    int m_labelHeight;
    QPen m_penRx{QColor(0, 211, 10), 1.5};
    QPen m_penTx{QColor(255, 57, 0), 1.5};
    QPen m_gridPen{QColor(48, 48, 48), 1, Qt::DotLine/*DashLine*/};
    QPen m_borderPen{QColor(48, 48, 48)};
    QPen m_labelPen{QColor(160, 160, 160)};
    qint64 getTrayVisibleMax(int visibleSamples) const;
    int m_updateIntervalNet;
    int m_maxGridDivisions;
    static constexpr double BYTES_PER_MB = 1024.0 * 1024.0;   // 1 048 576.0
    static inline constexpr std::array<int, 10> kHumanDivisors100 = {1, 2, 4, 5, 8, 10, 20, 25, 40, 50};
    bool m_useHumanDivisions = true;
signals:
    void settingsRequested(); // custom signal
    void aboutRequested(); // custom signal
    void hideRequested(QWidget *form); // custom signal
    void pauseStateChanged(); // custom signal

};

#endif // NETGRAPHWIDGET_H