#ifndef GRAPHFORM_H
#define GRAPHFORM_H

#include <QWidget>
#include <QSettings>
#include "netgraphwidget.h"

namespace Ui
{
class GraphForm;
}

class GraphForm : public QWidget
{
    Q_OBJECT

public:
    explicit GraphForm(QWidget *parent = nullptr);
    ~GraphForm();
    void setUnit(NetGraphWidget::UnitMode unit);
protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    Ui::GraphForm *ui;
    QSettings *settings;
    QPoint m_dragPosition;
    QPoint m_dragStartPosition;
    bool m_isDragging = false;
    bool m_isQuitting = false;
    bool m_geometryRestored = false;
    void updateX11SkipTaskbar();
    void setWindowOpacityPercent(int percent);

    void readSettings();
};

#endif // GRAPHFORM_H
