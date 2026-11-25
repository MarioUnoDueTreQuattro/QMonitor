#include "graphform.h"
#include "ui_graphform.h"
#include <QCloseEvent>
#include <QTimer>
#include <QWindow>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

GraphForm::GraphForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GraphForm)
{
    ui->setupUi(this);
    settings = new QSettings();
    ui->labelAvg->setText("");
    ui->labelMax->setText("");
    ui->labelCurrent->setText("");
    readSettings();
}

GraphForm::~GraphForm()
{
    delete ui;
}

void GraphForm::closeEvent(QCloseEvent *event)
{
    qDebug() << __PRETTY_FUNCTION__ << this->windowTitle();
    if (m_isQuitting)
    {
        if (isVisible())
        {
            settings->setValue("geometry" + this->windowTitle(), saveGeometry());
            //settings->setValue("windowState", saveState());
        }
        settings->setValue("WinVisible" + this->windowTitle(), isVisible());
        settings->sync();
        event->accept();
        return;
    }
    // if (m_trayIcon)
    // m_trayIcon->hide();
    // QMainWindow::closeEvent(event);
    // event->accept();
    settings->setValue("geometry" + this->windowTitle(), saveGeometry());
    //settings->setValue("windowState", saveState());
    settings->setValue("WinVisible" + this->windowTitle(), isVisible());
    settings->sync();
    hide();
    event->ignore();
}

NetGraphWidget *GraphForm::graphWidget() const
{
    return ui->netGraph;
}

void GraphForm::readSettings()
{
    qDebug() << __PRETTY_FUNCTION__;
    // m_bShowAvgMax = settings->value("ShowAvgMaxNet", true).toBool();
    // ui->labelAvg->setVisible(m_bShowAvgMax);
    // ui->labelMax->setVisible(m_bShowAvgMax);
    bool bTitle = settings->value("WindowHideTitlebar", false).toBool();
    setWindowFrameless(bTitle);
    int iWindowOpacity = settings->value("WindowOpacity", 100).toInt();
    setWindowOpacityPercent(iWindowOpacity);
    bool alwaysOnTop = settings->value("WindowAlwaysOnTop", false).toBool();
    setWindowAlwaysOnTop(alwaysOnTop);
    bool bHideInTaskbar = settings->value("WindowHideInTaskbar", false).toBool();
    setWindowShowInTaskbar(!bHideInTaskbar);
    // QString savedDevice = settings->value("NetworkDevice", "wlp2s0").toString();
    // m_iface = savedDevice;
    // setNetworkInterface(m_iface);
    // setNetGraphSettings();
    // Ripristina la geometria della finestra
    //restoreGeometry(settings->value("geometry" + this->windowTitle()).toByteArray());
    // Ripristina lo stato di barre degli strumenti e pannelli agganciati
    // restoreState(settings->value("windowState"+this->windowTitle()).toByteArray());
}

void GraphForm::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_dragPosition = event->position().toPoint();
        m_dragStartPosition = event->globalPosition().toPoint();
        m_isDragging = false;
        event->accept();
    }
    QWidget::mousePressEvent(event);
}

void GraphForm::mouseMoveEvent(QMouseEvent *event)
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
    QWidget::mouseMoveEvent(event);
}

void GraphForm::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!m_geometryRestored)
    {
        m_geometryRestored = true;
        restoreGeometry(settings->value("geometry" + this->windowTitle()).toByteArray());
    }
    QTimer::singleShot(200, this, [this]()
    {
        updateX11SkipTaskbar();
    });
    m_maxGridDivisions = settings->value("MaxGridDivisions" + this->windowTitle(), 4).toInt();
    ui->netGraph->setMaxGridDivisions(m_maxGridDivisions);
    // bool bIsVisible = settings->value("WinVisible" + this->windowTitle(), true).toBool();
    // if (!bIsVisible)
    // {
    //     QTimer::singleShot(1000, this, &GraphForm::hide);
    // }
}

void GraphForm::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_isDragging = false;
        event->accept();
    }
    QWidget::mouseReleaseEvent(event);
}

void GraphForm::setMaxGridDivisions(int newMaxGridDivisions)
{
    m_maxGridDivisions = newMaxGridDivisions;
    ui->netGraph->setMaxGridDivisions(m_maxGridDivisions);
}

void GraphForm::setMaxVal(qint64 newMaxVal)
{
    m_maxVal = newMaxVal;
    ui->netGraph->setMaxVal(m_maxVal);
}

void GraphForm::setColors(const QVector<QColor> &newColors)
{
    m_colors = newColors;
    ui->netGraph->setColors(m_colors);
}

void GraphForm::setSampleHistory(SampleHistory *newSampleHistory)
{
    m_sampleHistory = newSampleHistory;
    ui->netGraph->setSampleHistory(m_sampleHistory);
}

void GraphForm::updateX11SkipTaskbar()
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

void GraphForm::setWindowShowInTaskbar(bool showInTaskbar)
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

void GraphForm::setWindowAlwaysOnTop(bool enabled)
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

void GraphForm::setWindowFrameless(bool frameless)
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
        flags |= Qt::WindowTitleHint
            | Qt::WindowSystemMenuHint
            | Qt::WindowMinMaxButtonsHint
            | Qt::WindowCloseButtonHint;
    }
    setWindowFlags(flags);
    if (bVisible)
    {
        show();
        raise();
    }
}

void GraphForm::setWindowOpacityPercent(int percent)
{
    if (percent < 10) percent = 10;
    const int clamped = qBound(0, percent, 100);
    setWindowOpacity(clamped / 100.0);
}

void GraphForm::setUnit(NetGraphWidget::UnitMode unit)
{
    ui->netGraph->setUnitMode(unit);
}

