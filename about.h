#ifndef ABOUT_H
#define ABOUT_H

#include <QDialog>

namespace Ui
{
class AboutDialog;
}

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog();
    void setupWidgets();
    void adjustTextEditColor();
protected:
    void showEvent(QShowEvent *event) override;
private slots:
    void on_pushButtonOk_clicked();
    void on_pushButtonAboutQt_clicked();
    void adjustTextEditHeight();
private:
    Ui::AboutDialog *ui;
    QString getSystemInformationString();
    void centerOnParent();
    void centerOnScreen(QDialog *dialog);
};

#endif // ABOUT_H
