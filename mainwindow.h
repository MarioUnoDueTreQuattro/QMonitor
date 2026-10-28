#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "netgraphwidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateStats();

private:
    Ui::MainWindow *ui;
    QTimer *m_timer;

    QString m_iface;
    qint64 m_lastRx;
    qint64 m_lastTx;

    bool readNetDev(const QString &targetIface, qint64 &rxBytes, qint64 &txBytes);
    QString formatBytes(qint64 bytes);
};

#endif // MAINWINDOW_H