#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "netgraphwidget.h"
#include "samplehistory.h"
#include "cpureader.h"
#include <QMainWindow>
#include <QTimer>
#include <QSettings>
#include <QSystemTrayIcon>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
private slots:
    void readSettings();
    void updateStats();
    void on_btnPause_clicked();
    void on_pauseChanged();
    void on_btnReset_clicked();
    void on_actionSettings_triggered();
    void on_actionAbout_triggered();
    void settingsDialogAccepted();
    void trayIconNetActivated(QSystemTrayIcon::ActivationReason reason);
    void toggleMainWindow();
    void quitApplication();
private:
    Ui::MainWindow *ui;
    bool isAnyGraphEnabled() const;
    QPoint m_dragPosition;
    QPoint m_dragStartPosition;
    bool m_isDragging = false;
    QSystemTrayIcon *m_trayIconNet;
    QSystemTrayIcon *m_trayIconCPU;
    QSystemTrayIcon *m_trayIconRAM;
    QVector<QColor> *colors_net;
    QVector<QColor> *colors_cpu;
    QVector<QColor> *colors_ram;
    // QMenu *trayMenuNet;
    // QAction *actAbout;
    // QAction *actPause;
    // QAction *showHideAction;
    // QAction *quitAction;
    // QAction *actSettings;
    QAction *m_actToggleWindow;
    QAction *m_actPause;
    QAction *m_actSettings;
    QAction *m_actAbout;
    QAction *m_actQuit;
    QTimer *m_timerNet;
    QTimer *m_timerCPU;
    QTimer *m_timerRAM;
    QSettings *settings;
    QString m_iface;
    qint64 m_lastRx;
    qint64 m_lastTx;
    qint64 m_lastTime = 0;
    bool readNetDev(const QString &targetIface, qint64 &rxBytes, qint64 &txBytes);
    QString formatBytesNet(qint64 bytes);
    QString formatBytes(qint64 bytes);
    bool m_MonitorNet;
    bool m_MonitorCPU;
    bool m_MonitorMemory;
    int m_updateIntervalNet;
    int m_updateIntervalCPU;
    int m_updateIntervalRAM;
    void setNetGraphSettings();
    bool m_bShowAvgMax;
    bool m_isQuitting = false;
    void setWindowFrameless(bool frameless);
    void setWindowOpacityPercent(int percent);
    bool isWindowAlwaysOnTop() const;
    void setWindowAlwaysOnTop(bool enabled);
    void setWindowShowInTaskbar(bool show);
    void updateX11SkipTaskbar();
    SampleHistory *m_networkHistory;
    SampleHistory *m_cpuHistory;
    SampleHistory *m_memoryHistory;
    CpuReader m_cpuReader;
    void sampleCpu();
    void sampleMemory();
    qint64 m_memoryScaleBytes;
    void readColors();
    void createTrayActions();
    QMenu *createTrayMenu();
    void createTrayIcons();
    void setNetworkInterface(const QString &interfaceName);
    QByteArray m_netInterfaceKey;
    static constexpr double BYTES_PER_MB = 1024.0 * 1024.0;   // 1 048 576.0
};

#endif // MAINWINDOW_H