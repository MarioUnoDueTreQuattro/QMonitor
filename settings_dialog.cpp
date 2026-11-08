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
    settingsMgr = new QSettings();
    setupDialog();
    adjustSize();
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(saveCurrentTabIndex(int)));
}

SettingsDialog::~SettingsDialog()
{
    delete settingsMgr;
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
    bool bSingleInstance = settingsMgr->value("SingleInstance", true).toBool();
    ui->checkSingleInstance->setChecked(bSingleInstance);
    // Default: forza Dolphin se NON siamo su KDE/Plasma
    // const QString desktop = qEnvironmentVariable("XDG_CURRENT_DESKTOP").toLower();
    // const bool isKde = desktop.contains("kde") || desktop.contains("plasma");
    // const bool bForceDolphin = settingsMgr->value("ForceDolphin", !isKde).toBool();
    // ui->checkForceDolphin->setChecked(bForceDolphin);
}

void SettingsDialog::saveSettings()
{
    settingsMgr->setValue("SingleInstance", ui->checkSingleInstance->isChecked());
    settingsMgr->setValue("SettingsTab", ui->tabWidget->currentIndex());
    // settingsMgr->setValue("ForceDolphin", ui->checkForceDolphin->isChecked());
    settingsMgr->sync();
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
    QPoint savedPos = settingsMgr->value("SettingsPosition", QPoint(pos_x, pos_y)).toPoint();
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
    int iTab = settingsMgr->value("SettingsTab", 0).toInt();
    ui->tabWidget->setCurrentIndex(iTab);
}

void SettingsDialog::moveEvent(QMoveEvent *event)
{
    QWidget::moveEvent(event);
    //QSettings settings(QApplication::organizationName(), QApplication::applicationName());
    if (isVisible()) settingsMgr->setValue("SettingsPosition", pos());
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
    settingsMgr->setValue("SettingsTab", ui->tabWidget->currentIndex());
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
