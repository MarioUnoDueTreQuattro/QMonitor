#include "settings_dialog.h"
#include "ui_settings_dialog.h"
#include <QDebug>
#include <QScreen>
#include <QGuiApplication>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::MSWindowsFixedSizeDialogHint);
    setWindowTitle("Settings");
    settings = new QSettings();
    setupDialog();
    adjustSize();
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(saveCurrentTabIndex(int)));
}

SettingsDialog::~SettingsDialog()
{
    delete settings;
    delete ui;
}

void SettingsDialog::setupDialog()
{
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
    settings->setValue("ShowAvgMax", ui->checkShowAvgMax->isChecked());
    settings->setValue("SettingsTab", ui->tabWidget->currentIndex());
    // settingsMgr->setValue("ForceDolphin", ui->checkForceDolphin->isChecked());
    saveColors();
    settings->setValue("DrawModeLines", ui->radioButtonDrawModeLines->isChecked());
    settings->setValue("UpdateIntervalNet",ui->spinBoxInterval->value());
    settings->sync();
}

void SettingsDialog::loadSettings()
{
    QRect screenGeometry;
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen)
    {
        screenGeometry = screen->availableGeometry();
        // oppure usa screen->geometry() a seconda delle necessità
    }
    int pos_x = (screenGeometry.width() - this->width()) / 2;
    int pos_y = (screenGeometry.height() - this->height()) / 2;
    //QSettings settings(QApplication::organizationName(), QApplication::applicationName());
    QPoint savedPos = settings->value("SettingsPosition", QPoint(pos_x, pos_y)).toPoint();
    //QScreen *screen = QGuiApplication::screenAt(savedPos);
    if (screen)
    {
        move(savedPos);
    }
    else
    {
        // Se la posizione non è valida, centra la finestra
        move(QPoint(pos_x, pos_y));
    }
    int iTab = settings->value("SettingsTab", 0).toInt();
    ui->tabWidget->setCurrentIndex(iTab);
    bool bShowAvgMax = settings->value("ShowAvgMax", true).toBool();
    ui->checkShowAvgMax->setChecked(bShowAvgMax);
    bool bSingleInstance = settings->value("SingleInstance", true).toBool();
    ui->checkSingleInstance->setChecked(bSingleInstance);
    loadColors();
    bool bDrawModeLines = settings->value("DrawModeLines", true).toBool();
    if (bDrawModeLines) ui->radioButtonDrawModeLines->setChecked(true);
    else ui->radioButtonDrawModeBars->setChecked(true);
    ui->spinBoxInterval->setValue(settings->value("UpdateIntervalNet",2000).toInt());
}

void SettingsDialog::moveEvent(QMoveEvent *event)
{
    QWidget::moveEvent(event);
    //QSettings settings(QApplication::organizationName(), QApplication::applicationName());
    if (isVisible()) settings->setValue("SettingsPosition", pos());
}

void SettingsDialog::applyButtonClicked()
{
    qDebug() << __PRETTY_FUNCTION__ ;
    // on_buttonBox_clicked(btnApply);
}

void SettingsDialog::on_SettingsDialog_accepted()
{
    qDebug() << __PRETTY_FUNCTION__ ;
    saveSettings();
}

void SettingsDialog::saveCurrentTabIndex(int)
{
    settings->setValue("SettingsTab", ui->tabWidget->currentIndex());
}

void SettingsDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    qDebug() << __PRETTY_FUNCTION__ << button->objectName();
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
        QString storedName = settings->value(button->objectName(), defaultColor.name()).toString();
        button->setColor(QColor(storedName));
    }
    ui->spinBoxDownOpacity->setValue(settings->value("DownloadOpacity", 100).toInt());
    ui->spinBoxUpOpacity->setValue(settings->value("UploadOpacity", 100).toInt());
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
}
