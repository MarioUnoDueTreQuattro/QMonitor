#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_iface("wlp2s0") // Puoi renderlo configurabile o passarlo da argomento
    , m_lastRx(0)
    , m_lastTx(0)
{
    ui->setupUi(this);
    setWindowTitle(QString("QMonitor [%1]").arg(m_iface));

    // Timer di campionamento (1 secondo)
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::updateStats);
    m_timer->start(1000);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateStats()
{
    qint64 rx = 0, tx = 0;
    if (readNetDev(m_iface, rx, tx)) {
        qint64 rxSpeed = (m_lastRx > 0) ? (rx - m_lastRx) : 0;
        qint64 txSpeed = (m_lastTx > 0) ? (tx - m_lastTx) : 0;

        m_lastRx = rx;
        m_lastTx = tx;

        if (rxSpeed < 0) rxSpeed = 0;
        if (txSpeed < 0) txSpeed = 0;

        // Aggiorna l'etichetta testuale (assicurati di avere una QLabel di nome labelSpeed nel .ui)
        if (ui->labelSpeed) {
            ui->labelSpeed->setText(QString("DL: %1  |  UL: %2")
                .arg(formatBytes(rxSpeed))
                .arg(formatBytes(txSpeed)));
        }

        // Invia i dati al widget grafico promosso (chiamato netGraph nel .ui)
        if (ui->netGraph) {
            ui->netGraph->addData(rxSpeed, txSpeed);
        }
    }
}

bool MainWindow::readNetDev(const QString &targetIface, qint64 &rxBytes, qint64 &txBytes)
{
    QFile file("/proc/net/dev");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

    QTextStream in(&file);
    in.readLine();
    in.readLine();

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.startsWith(targetIface + ":")) {
            QStringList parts = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            if (parts.size() >= 10) {
                rxBytes = parts[1].toLongLong();
                txBytes = parts[9].toLongLong();
                return true;
            }
        }
    }
    return false;
}

QString MainWindow::formatBytes(qint64 bytes)
{
    if (bytes < 1024) return QString("%1 B/s").arg(bytes);
    if (bytes < 1024 * 1024) return QString("%1 KB/s").arg(bytes / 1024.0, 0, 'f', 1);
    return QString("%1 MB/s").arg(bytes / (1024.0 * 1024.0), 0, 'f', 1);
}