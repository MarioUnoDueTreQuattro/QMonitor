#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "netgraphwidget.h"
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
private slots:
    void readSettings();
    void updateStats();
    void on_btnPause_clicked();
    void on_pauseChanged( );
    void on_btnReset_clicked();
    void on_actionSettings_triggered();
    void settingsDialogAccepted();

    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void toggleMainWindow();
    void quitApplication();
private:
    Ui::MainWindow *ui;
    QSystemTrayIcon *m_trayIconNet;
    QMenu *trayMenuNet;
    QAction *actPause;
    QAction *showHideAction;
    QAction *quitAction;
    QAction *actSettings;
    QTimer *m_timer;
    QSettings *settings;
    QString m_iface;
    qint64 m_lastRx;
    qint64 m_lastTx;
    qint64 m_lastTime = 0;
    bool readNetDev(const QString &targetIface, qint64 &rxBytes, qint64 &txBytes);
    QString formatBytes(qint64 bytes);

    void setNetGraphSettings();
    bool m_bShowAvgMax;
    bool m_isQuitting = false;
};

#endif // MAINWINDOW_H