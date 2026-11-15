#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QSettings>

namespace Ui
{
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();
    void setupDialog();
private:
    Ui::SettingsDialog *ui;
    QSettings *settings;
    QDialogButtonBox *buttonBox;
    QPushButton *btnOk;
    QPushButton *btnApply;
    QPushButton *btnCancel;
    void saveSettings();
    void loadSettings();
    void loadColors();
    void saveColors();
private slots:
    void on_buttonBox_clicked(QAbstractButton *button);
    void on_SettingsDialog_accepted();
    void saveCurrentTabIndex(int);
    void applyButtonClicked();
    void on_checkBoxMonitorNet_checkStateChanged(const Qt::CheckState &arg1);
    void on_checkBoxMonitorCPU_checkStateChanged(const Qt::CheckState &arg1);
    void on_checkBoxMonitorMemory_checkStateChanged(const Qt::CheckState &arg1);
    void saveWindowPosition();

protected:
    void moveEvent(QMoveEvent *event) override;
signals:
    void applyClicked(); // custom signal
};

#endif // SETTINGS_DIALOG_H
