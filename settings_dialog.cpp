#include "settings_dialog.h"
#include "ui_settings_dialog.h"
#include <QDebug>
#include <QScreen>
#include <QTimer>
#include <QGuiApplication>
#include <QNetworkInterface>
#include <QNetworkAddressEntry>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::Dialog);
    adjustSize();
    //setWindowFlags(Qt::MSWindowsFixedSizeDialogHint);
    setWindowTitle("Settings");
    settings = new QSettings();
    setupDialog();
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(saveCurrentTabIndex(int)));
}

SettingsDialog::~SettingsDialog()
{
    delete settings;
    delete ui;
    // qDebug() << __PRETTY_FUNCTION__;
}

void SettingsDialog::setupDialog()
{
    loadNetworkDevices();
    loadSettings();
    // Connessioni standard: accepted -> accept, rejected -> reject
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    // Apply: gestisci il click del pulsante Apply esplicitamente
    // if (auto *btnApply = ui->buttonBox->button(QDialogButtonBox::Apply)) {
    // connect(btnApply, &QPushButton::clicked, this, &SettingsDialog::applyButtonClicked);
    // }
    // Default: forza Dolphin se NON siamo su KDE/Plasma
    // const QString desktop = qEnvironmentVariable("XDG_CURRENT_DESKTOP").toLower();
    // const bool isKde = desktop.contains("kde") || desktop.contains("plasma");
    // const bool bForceDolphin = settingsMgr->value("ForceDolphin", !isKde).toBool();
    // ui->checkForceDolphin->setChecked(bForceDolphin);
}

void SettingsDialog::saveSettings()
{
    settings->setValue("SingleInstance", ui->checkSingleInstance->isChecked());
    settings->setValue("ShowAvgMaxNet", ui->checkShowAvgMax->isChecked());
    settings->setValue("SettingsTab", ui->tabWidget->currentIndex());
    // settingsMgr->setValue("ForceDolphin", ui->checkForceDolphin->isChecked());
    saveColors();
    settings->setValue("DrawModeLinesNet", ui->radioButtonDrawModeLines->isChecked());
    settings->setValue("DrawModeLinesCPU", ui->radioButtonDrawModeLinesCPU->isChecked());
    settings->setValue("DrawModeLinesMem", ui->radioButtonDrawModeLinesMemory->isChecked());
    settings->setValue("UpdateIntervalNet", ui->spinBoxInterval->value());
    settings->setValue("UpdateIntervalCPU", ui->spinBoxIntervaCPU->value());
    settings->setValue("UpdateIntervalRAM", ui->spinBoxIntervalMemory->value());
    settings->setValue("MonitorNet", ui->checkBoxMonitorNet->isChecked());
    settings->setValue("MonitorCPU", ui->checkBoxMonitorCPU->isChecked());
    settings->setValue("MonitorMemory", ui->checkBoxMonitorMemory->isChecked());
    settings->setValue("WindowHideTitlebar", ui->checkBoxTitlebar->isChecked());
    settings->setValue("WindowOpacity", ui->spinBoxWindowOpacity->value());
    settings->setValue("WindowAlwaysOnTop", ui->checkBoxAlwaysOnTop->isChecked());
    settings->setValue("WindowHideInTaskbar", ui->checkBoxTaskbar->isChecked());
    settings->setValue("WindowBypassWM", ui->checkBoxWindowBypassWM->isChecked());
    settings->setValue("NetworkDevice", ui->comboNetworkDevice->currentData().toString());
    settings->setValue("TrayIconsSize", ui->spinBoxTrayIconsSize->value());
    settings->sync();
}

void SettingsDialog::loadSettings()
{
    QRect screenGeometry;
    QScreen *screen = QGuiApplication::primaryScreen();
    QPoint defaultPos;
    if (screen)
    {
        screenGeometry = screen->availableGeometry();
        defaultPos = QPoint((screenGeometry.width() - width()) / 2,
            (screenGeometry.height() - height()) / 2);
        // oppure usa screen->geometry() a seconda delle necessità
    }
    // int pos_x = (screenGeometry.width() - this->width()) / 2;
    // int pos_y = (screenGeometry.height() - this->height()) / 2;
    //QSettings settings(QApplication::organizationName(), QApplication::applicationName());
    //QPoint savedPos = settings->value("SettingsPosition", QPoint(pos_x, pos_y)).toPoint();
    QPoint savedPos = settings->value("SettingsPosition", defaultPos).toPoint();
    move(savedPos);
    //QScreen *screen = QGuiApplication::screenAt(savedPos);
    // if (screen)
    // {
    // move(savedPos);
    // qDebug() << __PRETTY_FUNCTION__<< " posX:"<<savedPos.x() << " - posY:" <<savedPos.y();
    // }
    // else
    // {
    //     // Se la posizione non è valida, centra la finestra
    // move(QPoint(pos_x, pos_y));
    // }
    int iTab = settings->value("SettingsTab", 0).toInt();
    ui->tabWidget->setCurrentIndex(iTab);
    bool bShowAvgMax = settings->value("ShowAvgMaxNet", true).toBool();
    ui->checkShowAvgMax->setChecked(bShowAvgMax);
    bool bSingleInstance = settings->value("SingleInstance", true).toBool();
    ui->checkSingleInstance->setChecked(bSingleInstance);
    loadColors();
    bool bDrawModeLines;
    bDrawModeLines = settings->value("DrawModeLinesNet", true).toBool();
    if (bDrawModeLines) ui->radioButtonDrawModeLines->setChecked(true);
    else ui->radioButtonDrawModeBars->setChecked(true);
    bDrawModeLines = settings->value("DrawModeLinesCPU", true).toBool();
    if (bDrawModeLines) ui->radioButtonDrawModeLinesCPU->setChecked(true);
    else ui->radioButtonDrawModeBarsCPU->setChecked(true);
    bDrawModeLines = settings->value("DrawModeLinesMem", true).toBool();
    if (bDrawModeLines) ui->radioButtonDrawModeLinesMemory->setChecked(true);
    else ui->radioButtonDrawModeBarsMemory->setChecked(true);
    ui->spinBoxInterval->setValue(settings->value("UpdateIntervalNet", 2000).toInt());
    ui->spinBoxIntervaCPU->setValue(settings->value("UpdateIntervalCPU", 2000).toInt());
    ui->spinBoxIntervalMemory->setValue(settings->value("UpdateIntervalRAM", 2000).toInt());
    ui->checkBoxMonitorNet->setChecked(settings->value("MonitorNet", true).toBool());
    ui->checkBoxMonitorCPU->setChecked(settings->value("MonitorCPU", true).toBool());
    ui->checkBoxMonitorMemory->setChecked(settings->value("MonitorMemory", true).toBool());
    on_checkBoxMonitorNet_checkStateChanged(ui->checkBoxMonitorNet->checkState());
    on_checkBoxMonitorCPU_checkStateChanged(ui->checkBoxMonitorCPU->checkState());
    on_checkBoxMonitorMemory_checkStateChanged(ui->checkBoxMonitorMemory->checkState());
    bool bTitle = settings->value("WindowHideTitlebar", false).toBool();
    ui->checkBoxTitlebar->setChecked(bTitle);
    bool bAlwaysOnTop = settings->value("WindowAlwaysOnTop", false).toBool();
    ui->checkBoxAlwaysOnTop->setChecked(bAlwaysOnTop);
    bool bHideInTaskbar = settings->value("WindowHideInTaskbar", false).toBool();
    ui->checkBoxTaskbar->setChecked(bHideInTaskbar);
    int iWindowOpacity = settings->value("WindowOpacity", 100).toInt();
    ui->spinBoxWindowOpacity->setValue(iWindowOpacity);
    bool bWindowBypassWM = settings->value("WindowBypassWM", false).toBool();
    ui->checkBoxWindowBypassWM->setChecked(bWindowBypassWM);
    QString savedDevice = settings->value("NetworkDevice").toString();
    int index = ui->comboNetworkDevice->findData(savedDevice);
    if (index >= 0) ui->comboNetworkDevice->setCurrentIndex(index);
    int iTraySize = settings->value("TrayIconsSize", 32).toInt();
    ui->spinBoxTrayIconsSize->setValue(iTraySize);
    // for (int i = 0; i < ui->comboNetworkDevice->count(); ++i)
    // {
    // qDebug() << "Text:" << ui->comboNetworkDevice->itemText(i)
    // << "Data:" << ui->comboNetworkDevice->itemData(i);
    // }
    // qDebug() << "Saved device:" << savedDevice;
    // qDebug() << "Found index:"
    // << ui->comboNetworkDevice->findData(savedDevice);
}

void SettingsDialog::saveWindowPosition()
{
    //settings->setValue("SettingsPosition", pos());
    settings->setValue("SettingsPosition", geometry().topLeft());
    // qDebug() << __PRETTY_FUNCTION__ << " posX:" << pos().x() << " - posY:" << pos().y();
}

void SettingsDialog::moveEvent(QMoveEvent *event)
{
    QWidget::moveEvent(event);
    //QSettings settings(QApplication::organizationName(), QApplication::applicationName());
    if (isVisible() && !isMinimized() && !isMaximized())
    {
        QTimer::singleShot(200, this, &SettingsDialog::saveWindowPosition);
    }
}

void SettingsDialog::applyButtonClicked()
{
    // qDebug() << __PRETTY_FUNCTION__ ;
    // on_buttonBox_clicked(btnApply);
}

void SettingsDialog::on_SettingsDialog_accepted()
{
    // qDebug() << __PRETTY_FUNCTION__ ;
    saveSettings();
}

void SettingsDialog::saveCurrentTabIndex(int)
{
    settings->setValue("SettingsTab", ui->tabWidget->currentIndex());
}

void SettingsDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    // qDebug() << __PRETTY_FUNCTION__ << button->objectName();
    QDialogButtonBox *buttonBox = qobject_cast<QDialogButtonBox *>(sender());
    if (!buttonBox) return;
    QDialogButtonBox::ButtonRole role = buttonBox->buttonRole(button);
    switch (role)
    {
        case QDialogButtonBox::ApplyRole:
            qDebug() << "Apply clicked -> apply changes without closing";
            //applySettings();
            //on_SettingsDialog_accepted();
            saveSettings();
            emit applyClicked();   // <-- EMIT CUSTOM SIGNAL
            break;
        // case QDialogButtonBox::AcceptRole:
        // qDebug() << "OK clicked -> apply + close";
        // applySettings();
        // accept();
        // break;
        // case QDialogButtonBox::RejectRole:
        // qDebug() << "Cancel clicked -> discard + close";
        // reject();
        // break;
        default:
            break;
    }
}

void SettingsDialog::loadColors()
{
    QColor defaultColor;
    QList<ColorButton *> buttons = findChildren<ColorButton *>();
    for (int i = 0; i < buttons.size(); ++i)
    {
        ColorButton *button = buttons.at(i);
        if (button->objectName() == "pushButtonColorDownload") defaultColor = QColor(0, 192, 0, 255);
        if (button->objectName() == "pushButtonColorUpload") defaultColor = QColor(255, 85, 0, 255);
        if (button->objectName() == "pushButtonCPU") defaultColor = QColor(170, 0, 0, 255);
        if (button->objectName() == "pushButtonColorRAM") defaultColor = QColor(0, 170, 255, 255);
        if (button->objectName() == "pushButtonColorSwap") defaultColor = QColor(255, 80, 255, 255);
        if (button->objectName() == "pushButtonColorZram") defaultColor = QColor(170, 170, 0, 255);
        QString storedName = settings->value(button->objectName(), defaultColor.name()).toString();
        button->setColor(QColor(storedName));
    }
    ui->spinBoxDownOpacity->setValue(settings->value("DownloadOpacity", 100).toInt());
    ui->spinBoxUpOpacity->setValue(settings->value("UploadOpacity", 100).toInt());
    ui->spinBoxCPUOpacity->setValue(settings->value("CPUOpacity", 100).toInt());
    ui->spinBoxRAMOpacity->setValue(settings->value("RAMOpacity", 100).toInt());
    ui->spinBoxSwapOpacity->setValue(settings->value("SwapOpacity", 100).toInt());
    ui->spinBoxZramOpacity->setValue(settings->value("ZRamOpacity", 100).toInt());
}

void SettingsDialog::saveColors()
{
    QList<ColorButton *> buttons = findChildren<ColorButton *>();
    for (int i = 0; i < buttons.size(); ++i)
    {
        ColorButton *button = buttons.at(i);
        settings->setValue(button->objectName(), button->color().name());
    }
    settings->setValue("DownloadOpacity", ui->spinBoxDownOpacity->value());
    settings->setValue("UploadOpacity", ui->spinBoxUpOpacity->value());
    settings->setValue("CPUOpacity", ui->spinBoxCPUOpacity->value());
    settings->setValue("RAMOpacity", ui->spinBoxRAMOpacity->value());
    settings->setValue("SwapOpacity", ui->spinBoxSwapOpacity->value());
    settings->setValue("ZRamOpacity", ui->spinBoxZramOpacity->value());
}

void SettingsDialog::on_checkBoxMonitorNet_checkStateChanged(const Qt::CheckState &arg1)
{
    const bool enabled = (arg1 == Qt::Checked);
    QWidget *page = ui->checkBoxMonitorNet->parentWidget();
    if (!page)
        return;
    const auto widgets = page->findChildren<QWidget *>();
    for (QWidget *w : widgets)
    {
        if (w == ui->checkBoxMonitorNet)
            continue;
        w->setEnabled(enabled);
    }
    // settings->setValue("MonitorNet", ui->checkBoxMonitorNet->isChecked());
}

void SettingsDialog::on_checkBoxMonitorCPU_checkStateChanged(const Qt::CheckState &arg1)
{
    const bool enabled = (arg1 == Qt::Checked);
    QWidget *page = ui->checkBoxMonitorCPU->parentWidget();
    if (!page)
        return;
    const auto widgets = page->findChildren<QWidget *>();
    for (QWidget *w : widgets)
    {
        if (w == ui->checkBoxMonitorCPU)
            continue;
        w->setEnabled(enabled);
    }
    // settings->setValue("MonitorCPU", enabled);
}

void SettingsDialog::on_checkBoxMonitorMemory_checkStateChanged(const Qt::CheckState &arg1)
{
    const bool enabled = (arg1 == Qt::Checked);
    QWidget *page = ui->checkBoxMonitorMemory->parentWidget();
    if (!page)
        return;
    const auto widgets = page->findChildren<QWidget *>();
    for (QWidget *w : widgets)
    {
        if (w == ui->checkBoxMonitorMemory)
            continue;
        w->setEnabled(enabled);
    }
}

void SettingsDialog::loadNetworkDevices()
{
    ui->comboNetworkDevice->clear();
    const QList<QNetworkInterface> interfaces =
        QNetworkInterface::allInterfaces();
    for (const QNetworkInterface &interface : interfaces)
    {
        // Ignore interfaces that are not usable.
        if (!(interface.flags() & QNetworkInterface::IsUp))
            continue;
        QString description = interface.humanReadableName();
        if (description.isEmpty())
            description = interface.name();
        QString text = QString("%1 (%2)")
            .arg(interface.name())
            .arg(description);
        ui->comboNetworkDevice->addItem(text, interface.name());
    }
}
