#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settings_dialog.h"
#include "about.h"
#include "memoryreader.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QSettings>
#include <QCloseEvent>
#include <QWindow>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <cstring>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_iface("wlp2s0") // Puoi renderlo configurabile o passarlo da argomento
    , m_lastRx(0)
    , m_lastTx(0)
{
    m_networkHistory = new SampleHistory(2, this);   // rx, tx (bytes/s)
    m_cpuHistory = new SampleHistory(1, this);   // total load (%)
    m_memoryHistory = new SampleHistory(3, this);   // ram, swap, zram (% of their total)
    colors_net = new QVector<QColor>();
    colors_cpu = new QVector<QColor>();
    colors_ram = new QVector<QColor>();
    ui->setupUi(this);
    setWindowTitle(QString("QMonitor [%1]").arg(m_iface));
    settings = new QSettings();
    // Timer di campionamento (1 secondo)
    m_updateIntervalNet = settings->value("UpdateIntervalNet", 2000).toInt();
    m_timer = new QTimer(this);
    m_timer->setTimerType(Qt::PreciseTimer);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::updateStats);
    m_timer->start(m_updateIntervalNet);
    ui->labelAvg->setText("");
    ui->labelMax->setText("");
    ui->labelSpeed->setText("");
    ui->labelSpeed->setVisible(false);
    readSettings();
    ui->btnPause->setVisible(false);
    ui->btnReset->setVisible(false);
    ui->horizontalSpacer_2->changeSize(0, 0); // setVisible(false);
    connect(ui->netGraph, &NetGraphWidget::settingsRequested, this, &MainWindow::on_actionSettings_triggered);
    connect(ui->netGraph, &NetGraphWidget::aboutRequested, this, &MainWindow::on_actionAbout_triggered);
    connect(ui->netGraph, &NetGraphWidget::hideRequested, this, &MainWindow::hide);
    connect(ui->netGraph, &NetGraphWidget::pauseStateChanged, this, &MainWindow::on_pauseChanged);
    m_trayIconNet = new QSystemTrayIcon(this);
    m_trayIconNet->setIcon(this->windowIcon());
    m_trayIconCPU = new QSystemTrayIcon(this);
    m_trayIconCPU->setIcon(this->windowIcon());
    m_trayIconRAM = new QSystemTrayIcon(this);
    m_trayIconRAM->setIcon(this->windowIcon());
    connect(m_trayIconNet, &QSystemTrayIcon::activated, this, &MainWindow::trayIconNetActivated);
    connect(m_trayIconCPU, &QSystemTrayIcon::activated, this, &MainWindow::trayIconNetActivated);
    connect(m_trayIconRAM, &QSystemTrayIcon::activated, this, &MainWindow::trayIconNetActivated);
    // trayMenuNet = new QMenu(this);
    // showHideAction = trayMenuNet->addAction("Hide/Show");
    // showHideAction->setIcon(QIcon::fromTheme("password-show-on"));
    // actPause = trayMenuNet->addAction(tr("Pause"));
    // actPause->setIcon(QIcon::fromTheme("media-playback-pause"));
    // actPause->setCheckable(true);
    // actPause->setChecked(false);
    // trayMenuNet->addSeparator();
    // actSettings = trayMenuNet->addAction(tr("Settings..."));
    // actSettings->setIcon(QIcon::fromTheme("configure"));
    // actAbout = trayMenuNet->addAction(tr("About..."));
    // actAbout->setIcon(QIcon::fromTheme("QMonitor").pixmap(32, 32));
    // trayMenuNet->addSeparator();
    // quitAction = trayMenuNet->addAction("Exit");
    // quitAction->setIcon(QIcon::fromTheme("application-exit"));
    // connect(actAbout, &QAction::triggered, this, &MainWindow::on_actionAbout_triggered);
    // connect(actSettings,
    //     &QAction::triggered,
    //     this,
    //     &MainWindow::on_actionSettings_triggered);
    // connect(showHideAction,
    //     &QAction::triggered,
    //     this,
    //     &MainWindow::toggleMainWindow);
    // connect(actPause,
    //     &QAction::triggered,
    //     this,
    //     &MainWindow::on_btnPause_clicked);
    // connect(quitAction,
    //     &QAction::triggered,
    //     this,
    //     &MainWindow::quitApplication);
    // m_trayIconNet->setContextMenu(trayMenuNet);
    // m_trayIconCPU->setContextMenu(trayMenuNet);
    // m_trayIconRAM->setContextMenu(trayMenuNet);
    // m_trayIconNet->show();
    // m_trayIconCPU->show();
    // m_trayIconRAM->show();
    createTrayIcons();
}

MainWindow::~MainWindow()
{
    delete settings;
    delete ui;
}

void MainWindow::createTrayActions()
{
    m_actToggleWindow = new QAction(tr("Hide/Show"), this);
    m_actToggleWindow->setIcon(QIcon::fromTheme("password-show-on"));

    m_actPause = new QAction(tr("Pause"), this);
    m_actPause->setIcon(QIcon::fromTheme("media-playback-pause"));
    m_actPause->setCheckable(true);
    m_actPause->setChecked(false);

    m_actSettings = new QAction(tr("Settings..."), this);
    m_actSettings->setIcon(QIcon::fromTheme("configure"));

    m_actAbout = new QAction(tr("About..."), this);
    m_actAbout->setIcon(QIcon::fromTheme("QMonitor").pixmap(32, 32));

    m_actQuit = new QAction(tr("Exit"), this);
    m_actQuit->setIcon(QIcon::fromTheme("application-exit"));

    connect(m_actToggleWindow, &QAction::triggered, this, &MainWindow::toggleMainWindow);
    connect(m_actPause,        &QAction::triggered, this, &MainWindow::on_btnPause_clicked);
    connect(m_actSettings,     &QAction::triggered, this, &MainWindow::on_actionSettings_triggered);
    connect(m_actAbout,        &QAction::triggered, this, &MainWindow::on_actionAbout_triggered);
    connect(m_actQuit,         &QAction::triggered, this, &MainWindow::quitApplication);
}

QMenu *MainWindow::createTrayMenu()
{
    // Each tray icon needs its own QMenu: sharing one menu between several
    // QSystemTrayIcon objects breaks the D-Bus menu export (No id for action).
    // The QActions themselves can safely be shared.
    QMenu *menu = new QMenu(this);
    menu->addAction(m_actToggleWindow);
    menu->addAction(m_actPause);
    menu->addSeparator();
    menu->addAction(m_actSettings);
    menu->addAction(m_actAbout);
    menu->addSeparator();
    menu->addAction(m_actQuit);
    return menu;
}

void MainWindow::createTrayIcons()
{
    createTrayActions();

    m_trayIconNet = new QSystemTrayIcon(this);
    m_trayIconCPU = new QSystemTrayIcon(this);
    m_trayIconRAM = new QSystemTrayIcon(this);

    m_trayIconNet->setIcon(windowIcon());
    m_trayIconCPU->setIcon(windowIcon());
    m_trayIconRAM->setIcon(windowIcon());

    connect(m_trayIconNet, &QSystemTrayIcon::activated, this, &MainWindow::trayIconNetActivated);
    connect(m_trayIconCPU, &QSystemTrayIcon::activated, this, &MainWindow::trayIconNetActivated);
    connect(m_trayIconRAM, &QSystemTrayIcon::activated, this, &MainWindow::trayIconNetActivated);

    m_trayIconNet->setContextMenu(createTrayMenu());
    m_trayIconCPU->setContextMenu(createTrayMenu());
    m_trayIconRAM->setContextMenu(createTrayMenu());

    m_trayIconNet->show();
    m_trayIconCPU->show();
    m_trayIconRAM->show();
}

void MainWindow::toggleMainWindow()
{
    if (isVisible())
    {
        hide();
        settings->setValue("MainWinVisible", false);
    }
    else
    {
        show();
        raise();
        activateWindow();
        settings->setValue("MainWinVisible", false);
    }
}

void MainWindow::quitApplication()
{
    m_isQuitting = true;
    qApp->quit();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_isQuitting)
    {
        if (true)//(isVisible())
        {
            settings->setValue("geometry", saveGeometry());
            settings->setValue("windowState", saveState());
        }
        settings->setValue("MainWinVisible", isVisible());
        event->accept();
        return;
    }
    // if (m_trayIcon)
    // m_trayIcon->hide();
    // QMainWindow::closeEvent(event);
    // event->accept();
    settings->setValue("geometry", saveGeometry());
    settings->setValue("windowState", saveState());
    settings->setValue("MainWinVisible", false);
    hide();
    event->ignore();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_dragPosition = event->position().toPoint();
        m_dragStartPosition = event->globalPosition().toPoint();
        m_isDragging = false;
        event->accept();
    }
    QMainWindow::mousePressEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        if (!m_isDragging)
        {
            // Check if movement exceeds 4 pixels threshold
            QPoint currentGlobal = event->globalPosition().toPoint();
            if ((currentGlobal - m_dragStartPosition).manhattanLength() > 4)
            {
                m_isDragging = true;
            }
        }
        if (m_isDragging)
        {
            move(event->globalPosition().toPoint() - m_dragPosition);
            event->accept();
        }
    }
    QMainWindow::mouseMoveEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_isDragging = false;
        event->accept();
    }
    QMainWindow::mouseReleaseEvent(event);
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    // Delay the EWMH state enforcement slightly to ensure Openbox has fully mapped the window
    QTimer::singleShot(200, this, [this]()
    {
        updateX11SkipTaskbar();
    });
}

void MainWindow::updateX11SkipTaskbar()
{
    if (!isVisible())
        return;
    createWinId();
    if (QWindow *nativeWin = windowHandle())
    {
        Display *display = XOpenDisplay(nullptr);
        if (display)
        {
            Atom atomState = XInternAtom(display, "_NET_WM_STATE", False);
            Atom atomSkip = XInternAtom(display, "_NET_WM_STATE_SKIP_TASKBAR", False);
            if (atomState != None && atomSkip != None)
            {
                // Determine target state based on window flags (Qt::Tool means hidden from taskbar)
                bool showInTaskbar = !windowFlags().testFlag(Qt::Tool);
                XEvent xev;
                std::memset(&xev, 0, sizeof(xev));
                xev.type = ClientMessage;
                xev.xclient.window = nativeWin->winId();
                xev.xclient.message_type = atomState;
                xev.xclient.format = 32;
                // action: 1 = add property (hide from taskbar), 0 = remove property (show in taskbar)
                xev.xclient.data.l[0] = showInTaskbar ? 0 : 1;
                xev.xclient.data.l[1] = atomSkip;
                xev.xclient.data.l[2] = 0;
                xev.xclient.data.l[3] = 0;
                xev.xclient.data.l[4] = 0;
                XSendEvent(display, DefaultRootWindow(display), False,
                    SubstructureRedirectMask | SubstructureNotifyMask, &xev);
                XFlush(display);
            }
            XCloseDisplay(display);
        }
    }
    int iWindowOpacity = settings->value("WindowOpacity", 100).toInt();
    setWindowOpacityPercent(100);
    setWindowOpacityPercent(iWindowOpacity);
}

void MainWindow::readSettings()
{
    m_bShowAvgMax = settings->value("ShowAvgMaxNet", true).toBool();
    ui->labelAvg->setVisible(m_bShowAvgMax);
    ui->labelMax->setVisible(m_bShowAvgMax);
    bool bIsVisible = settings->value("MainWinVisible", true).toBool();
    if (!bIsVisible)
    {
        QTimer::singleShot(500, this, &MainWindow::hide);
    }
    bool bTitle = settings->value("WindowHideTitlebar", false).toBool();
    setWindowFrameless(bTitle);
    int iWindowOpacity = settings->value("WindowOpacity", 100).toInt();
    setWindowOpacityPercent(iWindowOpacity);
    bool alwaysOnTop = settings->value("WindowAlwaysOnTop", false).toBool();
    setWindowAlwaysOnTop(alwaysOnTop);
    bool bHideInTaskbar = settings->value("WindowHideInTaskbar", false).toBool();
    setWindowShowInTaskbar(!bHideInTaskbar);
    setNetGraphSettings();
    // Ripristina la geometria della finestra
    restoreGeometry(settings->value("geometry").toByteArray());
    // Ripristina lo stato di barre degli strumenti e pannelli agganciati
    restoreState(settings->value("windowState").toByteArray());
}

bool MainWindow::isWindowAlwaysOnTop() const
{
    return windowFlags().testFlag(Qt::WindowStaysOnTopHint);
}

void MainWindow::setWindowAlwaysOnTop(bool enabled)
{
    const bool bWindowBypassWM = settings->value("WindowBypassWM", false).toBool();
    const bool wasVisible = isVisible();
    Qt::WindowFlags flags = windowFlags();
    if (enabled)
    {
        flags |= Qt::WindowStaysOnTopHint;
        if (bWindowBypassWM)
        {
            flags |= Qt::BypassWindowManagerHint;
        }
        else
        {
            flags &= ~Qt::BypassWindowManagerHint;
        }
    }
    else
    {
        flags &= ~Qt::WindowStaysOnTopHint;
        flags &= ~Qt::BypassWindowManagerHint;
    }
    setWindowFlags(flags);
    // Necessario dopo setWindowFlags()
    if (wasVisible)
    {
        show();
        raise();
        //activateWindow();
    }
}
/*
void MainWindow::setWindowAlwaysOnTop(bool enabled)
{
    const bool wasVisible = isVisible();
    Qt::WindowFlags flags = windowFlags();
    if (enabled)
    {
        flags |= Qt::WindowStaysOnTopHint;
        // flags |= Qt::BypassWindowManagerHint;
    }
    else
    {
        flags &= ~Qt::WindowStaysOnTopHint;
        // flags &= ~Qt::BypassWindowManagerHint;
    }
    setWindowFlags(flags);
    if (wasVisible)
    {
        show();
        raise();
    }
    // Enforce EWMH _NET_WM_STATE_ABOVE on X11 / Openbox to stay above full-screen windows
    if (isVisible())
    {
        createWinId();
        if (QWindow *nativeWin = windowHandle())
        {
            Display *display = XOpenDisplay(nullptr);
            if (display)
            {
                Atom atomState = XInternAtom(display, "_NET_WM_STATE", False);
                Atom atomAbove = XInternAtom(display, "_NET_WM_STATE_ABOVE", False);
                if (atomState != None && atomAbove != None)
                {
                    XEvent xev;
                    std::memset(&xev, 0, sizeof(xev));
                    xev.type = ClientMessage;
                    xev.xclient.window = nativeWin->winId();
                    xev.xclient.message_type = atomState;
                    xev.xclient.format = 32;
                    // action: 1 = add property (always on top), 0 = remove property (normal)
                    xev.xclient.data.l[0] = enabled ? 1 : 0;
                    xev.xclient.data.l[1] = atomAbove;
                    xev.xclient.data.l[2] = 0;
                    xev.xclient.data.l[3] = 0;
                    xev.xclient.data.l[4] = 0;
                    XSendEvent(display, DefaultRootWindow(display), False,
                        SubstructureRedirectMask | SubstructureNotifyMask, &xev);
                    XFlush(display);
                }
                XCloseDisplay(display);
            }
        }
    }
}
*/
void MainWindow::setWindowOpacityPercent(int percent)
{
    if (percent < 10) percent = 10;
    const int clamped = qBound(0, percent, 100);
    setWindowOpacity(clamped / 100.0);
}
void MainWindow::setWindowFrameless(bool frameless)
{
    const bool bVisible = isVisible();
    Qt::WindowFlags flags = windowFlags();
    if (frameless)
    {
        flags |= Qt::FramelessWindowHint;
    }
    else
    {
        flags &= ~Qt::FramelessWindowHint;
        // Facoltativo: ripristina una title bar standard
        // flags |= Qt::WindowTitleHint |
        // Qt::WindowSystemMenuHint |
        // Qt::WindowMinMaxButtonsHint |
        // Qt::WindowCloseButtonHint;
    }
    setWindowFlags(flags);
    if (bVisible)
    {
        show();
        raise();
    }
}
void MainWindow::setWindowShowInTaskbar(bool showInTaskbar)
{
    const bool wasVisible = isVisible();
    Qt::WindowFlags flags = windowFlags();
    // Clear only window type bits
    flags &= ~Qt::WindowType_Mask;
    if (showInTaskbar)
    {
        flags |= Qt::Window;
    }
    else
    {
        flags |= Qt::Tool;
    }
    setWindowFlags(flags);
    if (wasVisible)
    {
        show();
        raise();
    }
    updateX11SkipTaskbar();
}
// void MainWindow::setWindowShowInTaskbar(bool showInTaskbar)
// {
// const bool wasVisible = isVisible();
// Qt::WindowFlags flags = windowFlags();
//     // Rimuove il tipo corrente, senza cancellare gli hint
// flags &= ~Qt::WindowType_Mask;
// if (showInTaskbar)
// flags |= Qt::Window;
// else
// flags |= Qt::Tool;
// setWindowFlags(flags);
// if (wasVisible)
// {
// show();
// raise();
// }
// }

void MainWindow::sampleCpu()
{
    QVector<qint64> values;
    values << m_cpuReader.readLoadPercent();
    m_cpuHistory->addSample(values);   // scale is fixed at 100
    //qDebug() << __PRETTY_FUNCTION__ << values;
}

void MainWindow::sampleMemory()
{
    MemorySnapshot snapshot = MemoryReader::read(false);
    // Scale is stored as a member, read by the widget and by the tray renderer
    m_memoryScaleBytes = snapshot.maxTotalBytes;
    QVector<qint64> values;
    values << snapshot.ramUsedBytes << snapshot.swapUsedBytes << snapshot.zramUsedBytes;
    m_memoryHistory->addSample(values);   // emits historyChanged() after the scale is updated
}

void MainWindow::readColors()
{
    colors_cpu->clear();
    colors_ram->clear();
    QColor newColor;
    QColor defaultColor = QColor(255, 0, 0, 255);
    defaultColor = QColor(settings->value("pushButtonCPU", defaultColor.name()).toString());
    int iAlplha = settings->value("CPUOpacity", 100).toInt();
    iAlplha = qRound(iAlplha * 255.0 / 100.0);
    defaultColor.setAlpha(iAlplha);
    colors_cpu->append(defaultColor);
    defaultColor = QColor(settings->value("pushButtonColorRAM", defaultColor.name()).toString());
    iAlplha = settings->value("RAMOpacity", 100).toInt();
    iAlplha = qRound(iAlplha * 255.0 / 100.0);
    defaultColor.setAlpha(iAlplha);
    colors_ram->append(defaultColor);
    defaultColor = QColor(settings->value("pushButtonColorSwap", defaultColor.name()).toString());
    iAlplha = settings->value("SwapOpacity", 100).toInt();
    iAlplha = qRound(iAlplha * 255.0 / 100.0);
    defaultColor.setAlpha(iAlplha);
    colors_ram->append(defaultColor);
    defaultColor = QColor(settings->value("pushButtonColorZram", defaultColor.name()).toString());
    iAlplha = settings->value("ZRamOpacity", 100).toInt();
    iAlplha = qRound(iAlplha * 255.0 / 100.0);
    defaultColor.setAlpha(iAlplha);
    colors_ram->append(defaultColor);
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
            QString s_rxSpeed = formatBytesNet(rxSpeed);
            QString s_txSpeed = formatBytesNet(txSpeed);
            // 1. Aggiornamento della velocità istantanea
            // Inseriamo prima il punto nello storico del widget grafico
            if (ui->netGraph)
            {
                ui->netGraph->addData(rxSpeed, txSpeed);
                //m_trayIcon->showMessage(QString(""),QString("Down: %1\nUp: %2").arg(formatBytes(rxSpeed)).arg(formatBytes(txSpeed)),QSystemTrayIcon::Information,5000);
                m_trayIconNet->setToolTip(QString("Down: %1\nUp: %2") .arg(s_rxSpeed).arg(s_txSpeed));
            }
            if (isVisible())
            {
                if (ui->labelSpeed)
                {
                    // ui->labelSpeed->setText(QString("DL: %1  |  UL: %2")
                    // .arg(formatBytes(rxSpeed))
                    // .arg(formatBytes(txSpeed)));
                    ui->statusbar->showMessage(QString("DL: %1 - UL: %2")
                        .arg(s_rxSpeed)
                        .arg(s_txSpeed));
                }
                // 2. Aggiornamento della velocità media (prelevata dallo storico del grafico)
                if (m_bShowAvgMax)
                {
                    if (ui->labelAvg)
                    {
                        ui->labelAvg->setText(QString("Avg DL: %1 - UL: %2")
                            .arg(formatBytesNet(ui->netGraph->getAvgRx()))
                            .arg(formatBytesNet(ui->netGraph->getAvgTx())));
                    }
                    // 3. Aggiornamento della velocità massima (picco storico)
                    if (ui->labelMax)
                    {
                        ui->labelMax->setText(QString("Max DL: %1 - UL: %2")
                            .arg(formatBytesNet(ui->netGraph->getMaxRx()))
                            .arg(formatBytesNet(ui->netGraph->getMaxTx())));
                    }
                }
            }
            m_trayIconNet->setIcon(ui->netGraph->renderTrayIcon(24));
            sampleCpu();
            m_trayIconCPU->setIcon(ui->netGraph->renderTrayPixmap(m_cpuHistory, colors_cpu, 100, 24));
            m_trayIconCPU->setToolTip(QString("CPU load: %1%") .arg(m_cpuHistory->latestValue(0)));
            sampleMemory();
            m_trayIconRAM->setIcon(ui->netGraph->renderTrayPixmap(m_memoryHistory, colors_ram, m_memoryScaleBytes, 24));
            QString sMemory = QString("RAM used: %1\n").arg(formatBytes(m_memoryHistory->latestValue(0)));
            sMemory.append(QString("Swap used: %1\n").arg(formatBytes(m_memoryHistory->latestValue(1))));
            sMemory.append(QString("ZRam used: %1").arg(formatBytes(m_memoryHistory->latestValue(2))));
            m_trayIconRAM->setToolTip(sMemory);
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

bool MainWindow::readNetDev(const QString & targetIface, qint64 & rxBytes, qint64 & txBytes)
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
QString MainWindow::formatBytesNet(qint64 bytes)
{
    if (bytes < 1024) return QString("%1 B/s").arg(bytes);
    if (bytes < 1024 * 1024) return QString("%1 KB/s").arg(bytes / 1024.0, 0, 'f', 1);
    return QString("%1 MB/s").arg(bytes / (1024.0 * 1024.0), 0, 'f', 1);
}

QString MainWindow::formatBytes(qint64 bytes)
{
    return QString("%1 MB").arg(bytes / (1024.0 * 1024.0), 0, 'f', 0);
}

void MainWindow::on_btnPause_clicked()
{
    if (ui->netGraph->paused() == false)
        // if (m_timer->isActive())
    {
        //m_timer->stop();
        ui->btnPause->setText("Resume");
        ui->netGraph->setPaused(true);
        m_actPause->setText("Resume");
        m_actPause->setChecked(true);
    }
    else
    {
        //m_timer->start(2000);
        ui->btnPause->setText("Pause");
        ui->netGraph->setPaused(false);
        m_actPause->setText("Pause");
        m_actPause->setChecked(false);
        // Importante: azzeriamo i contatori precedenti alla ripresa,
        // altrimenti il primo delta dopo la pausa calcolerebbe un tempo enorme o un salto sballato.
        m_lastRx = 0;
        m_lastTx = 0;
    }
}

void MainWindow::on_pauseChanged()
{
    if (ui->netGraph->paused() == false) m_actPause->setText("Pause");
    else m_actPause->setText("Resume");
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
    m_updateIntervalNet = settings->value("UpdateIntervalNet", 2000).toInt();
    m_timer->setInterval(m_updateIntervalNet);
    ui->netGraph->setUpdateIntervalNet(m_updateIntervalNet);
    QColor defaultColor = QColor(0, 192, 0, 255);
    defaultColor = QColor(settings->value("pushButtonColorDownload", defaultColor.name()).toString());
    int iAlplha = settings->value("DownloadOpacity", 100).toInt();
    iAlplha = qRound(iAlplha * 255.0 / 100.0);
    defaultColor.setAlpha(iAlplha);
    ui->netGraph->setDownloadColor(defaultColor);
    defaultColor = QColor(255, 85, 0, 255);
    defaultColor = QColor(settings->value("pushButtonColorUpload", defaultColor.name()).toString());
    iAlplha = settings->value("UploadOpacity", 100).toInt();
    iAlplha = qRound(iAlplha * 255.0 / 100.0);
    defaultColor.setAlpha(iAlplha);
    ui->netGraph->setUploadColor(defaultColor);
    bool bDrawModeLines = settings->value("DrawModeLinesNet", true).toBool();
    //if (bDrawModeLines) ui->netGraph->setDrawMode(0); else ui->netGraph->setDrawMode(1);
    ui->netGraph->setDrawMode(bDrawModeLines ? NetGraphWidget::DrawModeLines : NetGraphWidget::DrawModeBars);
    readColors();
}
void MainWindow::settingsDialogAccepted()
{
    qDebug() << __PRETTY_FUNCTION__;
    m_bShowAvgMax = settings->value("ShowAvgMaxNet", true).toBool();
    ui->labelAvg->setVisible(m_bShowAvgMax);
    ui->labelMax->setVisible(m_bShowAvgMax);
    // ui->horizontalLayout->invalidate();
    // ui->horizontalLayout->parentWidget()->adjustSize();
    // ui->horizontalLayout->parentWidget()->setVisible(false);
    // ui->horizontalLayout->parentWidget()->setVisible(true);
    bool bTitle = settings->value("WindowHideTitlebar", false).toBool();
    setWindowFrameless(bTitle);
    int iWindowOpacity = settings->value("WindowOpacity", 100).toInt();
    setWindowOpacityPercent(iWindowOpacity);
    bool alwaysOnTop = settings->value("WindowAlwaysOnTop", false).toBool();
    setWindowAlwaysOnTop(alwaysOnTop);
    bool bHideInTaskbar = settings->value("WindowHideInTaskbar", false).toBool();
    setWindowShowInTaskbar(!bHideInTaskbar);
    setNetGraphSettings();
}
void MainWindow::trayIconNetActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger)
    {
        if (isVisible())
        {
            hide();
        }
        else
        {
            show();
            raise();
            activateWindow();
        }
    }
}
void MainWindow::on_actionAbout_triggered()
{
    AboutDialog aboutDlg(this);
    aboutDlg.exec();
}