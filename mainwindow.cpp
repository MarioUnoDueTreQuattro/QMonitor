#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settings_dialog.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QSettings>
#include <QCloseEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_iface("wlp2s0") // Puoi renderlo configurabile o passarlo da argomento
    , m_lastRx(0)
    , m_lastTx(0)
{
    ui->setupUi(this);
    setWindowTitle(QString("QMonitor [%1]").arg(m_iface));
    settings = new QSettings();
    // Timer di campionamento (1 secondo)
    m_timer = new QTimer(this);
    m_timer->setTimerType(Qt::PreciseTimer);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::updateStats);
    m_timer->start(2000);
    ui->labelAvg->setText("");
    ui->labelMax->setText("");
    ui->labelSpeed->setText("");
    ui->labelSpeed->setVisible(false);
    readSettings();
    ui->btnPause->setVisible(false);
    ui->btnReset->setVisible(false);
    ui->horizontalSpacer_2->changeSize(0, 0); // setVisible(false);
    connect(ui->netGraph, &NetGraphWidget::settingsRequested, this, &MainWindow::on_actionSettings_triggered);
    // m_trayIcon=new QSystemTrayIcon(this);
    // m_trayIcon->setIcon(this->windowIcon());
    // m_trayIcon->show();
}

MainWindow::~MainWindow()
{
    delete settings;
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // Salva posizione, dimensioni e stato massimizzato/schermo intero
    settings->setValue("geometry", saveGeometry());
    // Salva la disposizione delle barre degli strumenti e dei pannelli agganciati (necessario objectName)
    settings->setValue("windowState", saveState());
    QMainWindow::closeEvent(event);
}

void MainWindow::readSettings()
{
    // Ripristina la geometria della finestra
    restoreGeometry(settings->value("geometry").toByteArray());
    // Ripristina lo stato di barre degli strumenti e pannelli agganciati
    restoreState(settings->value("windowState").toByteArray());
    m_bShowAvgMax = settings->value("ShowAvgMax", true).toBool();
    ui->labelAvg->setVisible(m_bShowAvgMax);
    ui->labelMax->setVisible(m_bShowAvgMax);
    setNetGraphSettings();
}

void MainWindow::updateStats()
{
    bool graphIsPaused = ui->netGraph->paused();
    if (graphIsPaused) return;
    qint64 rx = 0, tx = 0;
    if (readNetDev(m_iface, rx, tx))
    {
        qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
        if (m_lastRx > 0 && m_lastTime > 0)
        {
            qint64 elapsedMs = currentTime - m_lastTime;
            // Ignoriamo chiamate duplicate troppo ravvicinate (< 400ms)
            if (elapsedMs < 400) return;
            double seconds = elapsedMs / 1000.0;
            qint64 rxDiff = rx - m_lastRx;
            qint64 txDiff = tx - m_lastTx;
            qint64 rxSpeed = (rxDiff > 0 && seconds > 0) ? static_cast<qint64>(rxDiff / seconds) : 0;
            qint64 txSpeed = (txDiff > 0 && seconds > 0) ? static_cast<qint64>(txDiff / seconds) : 0;
            m_lastRx = rx;
            m_lastTx = tx;
            m_lastTime = currentTime;
            // 1. Aggiornamento della velocità istantanea
            if (ui->labelSpeed)
            {
                // ui->labelSpeed->setText(QString("DL: %1  |  UL: %2")
                // .arg(formatBytes(rxSpeed))
                // .arg(formatBytes(txSpeed)));
                ui->statusbar->showMessage(QString("DL: %1 - UL: %2")
                    .arg(formatBytes(rxSpeed))
                    .arg(formatBytes(txSpeed)));
            }
            // Inseriamo prima il punto nello storico del widget grafico
            if (ui->netGraph)
            {
                ui->netGraph->addData(rxSpeed, txSpeed);
            }
            // 2. Aggiornamento della velocità media (prelevata dallo storico del grafico)
            if (m_bShowAvgMax)
            {
                if (ui->labelAvg)
                {
                    ui->labelAvg->setText(QString("Avg DL: %1 - UL: %2")
                        .arg(formatBytes(ui->netGraph->getAvgRx()))
                        .arg(formatBytes(ui->netGraph->getAvgTx())));
                }
                // 3. Aggiornamento della velocità massima (picco storico)
                if (ui->labelMax)
                {
                    ui->labelMax->setText(QString("Max DL: %1 - UL: %2")
                        .arg(formatBytes(ui->netGraph->getMaxRx()))
                        .arg(formatBytes(ui->netGraph->getMaxTx())));
                }
            }
        }
        else
        {
            // Prima esecuzione: inizializzazione dei riferimenti
            m_lastRx = rx;
            m_lastTx = tx;
            m_lastTime = currentTime;
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
    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed();
        if (line.startsWith(targetIface + ":"))
        {
            QStringList parts = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            if (parts.size() >= 10)
            {
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

void MainWindow::on_btnPause_clicked()
{
    if (m_timer->isActive())
    {
        m_timer->stop();
        ui->btnPause->setText("Resume");
    }
    else
    {
        m_timer->start(2000);
        ui->btnPause->setText("Pause");
        // Importante: azzeriamo i contatori precedenti alla ripresa,
        // altrimenti il primo delta dopo la pausa calcolerebbe un tempo enorme o un salto sballato.
        m_lastRx = 0;
        m_lastTx = 0;
    }
}

void MainWindow::on_btnReset_clicked()
{
    // Puliamo il grafico
    ui->netGraph->resetGraph();
    // Azzeriamo anche i contatori di rete per ripartire puliti con il prossimo delta
    m_lastRx = 0;
    m_lastTx = 0;
}

void MainWindow::on_actionSettings_triggered()
{
    SettingsDialog settingsDialog;
    connect(&settingsDialog, SIGNAL(accepted()), this, SLOT(settingsDialogAccepted()));
    connect(&settingsDialog, SIGNAL(applyClicked()), this, SLOT(settingsDialogAccepted()));
    settingsDialog.exec();
    return;
}

void MainWindow::setNetGraphSettings()
{
    QColor defaultColor = QColor(0, 192, 0, 255);
    ui->netGraph->setDownloadColor(settings->value("pushButtonColorDownload", defaultColor.name()).toString());
    defaultColor = QColor(255, 85, 0, 255);
    ui->netGraph->setUploadColor(settings->value("pushButtonColorUpload", defaultColor.name()).toString());
    bool bDrawModeLines = settings->value("DrawModeLines", true).toBool();
    if (bDrawModeLines) ui->netGraph->setDrawMode(0); else ui->netGraph->setDrawMode(1);

}

void MainWindow::settingsDialogAccepted()
{
    qDebug() << __PRETTY_FUNCTION__;
    m_bShowAvgMax = settings->value("ShowAvgMax", true).toBool();
    ui->labelAvg->setVisible(m_bShowAvgMax);
    ui->labelMax->setVisible(m_bShowAvgMax);
    // ui->horizontalLayout->invalidate();
    // ui->horizontalLayout->parentWidget()->adjustSize();
    // ui->horizontalLayout->parentWidget()->setVisible(false);
    // ui->horizontalLayout->parentWidget()->setVisible(true);
    setNetGraphSettings();
}

