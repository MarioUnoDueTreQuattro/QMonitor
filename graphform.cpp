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


}

GraphForm::~GraphForm()
{
    delete ui;
}

void GraphForm::closeEvent(QCloseEvent *event)
{
    if (m_isQuitting)
    {
        if (true)//(isVisible())
        {
            settings->setValue("geometry", saveGeometry());
            //settings->setValue("windowState", saveState());
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
    //settings->setValue("windowState", saveState());
    settings->setValue("MainWinVisible", false);
    hide();
    event->ignore();
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

void GraphForm::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_isDragging = false;
        event->accept();
    }
    QWidget::mouseReleaseEvent(event);
}

void GraphForm::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    // Delay the EWMH state enforcement slightly to ensure Openbox has fully mapped the window
    QTimer::singleShot(200, this, [this]()
    {
        //updateX11SkipTaskbar();
    });
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

void GraphForm::setWindowOpacityPercent(int percent)
{
    if (percent < 10) percent = 10;
    const int clamped = qBound(0, percent, 100);
    setWindowOpacity(clamped / 100.0);
}
