#include "colorbutton.h"

#include <QColorDialog>
#include <QIcon>
#include <QPixmap>

ColorButton::ColorButton(QWidget *parent)
    : QPushButton(parent),
      m_color(Qt::gray)
{
    connect(this, SIGNAL(clicked()), this, SLOT(chooseColor()));
    updateSwatch();
}

QColor ColorButton::color() const
{
    return m_color;
}

void ColorButton::setColor(const QColor &newColor)
{
    if (!newColor.isValid() || newColor == m_color)
        return;

    m_color = newColor;
    updateSwatch();
    emit colorChanged(m_color);
}

void ColorButton::chooseColor()
{
    QColor chosenColor = QColorDialog::getColor(m_color, this, tr("Select a color"));
    if (chosenColor.isValid())
        setColor(chosenColor);
}

void ColorButton::updateSwatch()
{
    // Fixed explicit size: does not depend on layout timing.
    // The icon works with any style (Fusion, Breeze, Kvantum...).
    QPixmap swatch(24, 16);
    swatch.fill(m_color);
    setIcon(QIcon(swatch));
    setIconSize(swatch.size());
    setToolTip(m_color.name());
}