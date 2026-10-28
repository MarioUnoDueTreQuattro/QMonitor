#include "mainwindow.h"

#include <QApplication>
#include <QCoreApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("andreag");
    QCoreApplication::setApplicationName("QMonitor");
    QGuiApplication::setDesktopFileName("QMonitor");
    a.setWindowIcon(QIcon::fromTheme("QMonitor"));
    MainWindow w;
    w.show();
    return QApplication::exec();
}
