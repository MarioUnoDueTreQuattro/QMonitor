#ifndef COLORBUTTON_H
#define COLORBUTTON_H

#include <QPushButton>
#include <QColor>

// Flat color swatch button: click opens QColorDialog, no text needed.
class ColorButton : public QPushButton
{
    Q_OBJECT

public:
    explicit ColorButton(QWidget *parent = nullptr);

    QColor color() const;
    void setColor(const QColor &newColor);

signals:
    void colorChanged(const QColor &newColor);

private slots:
    void chooseColor();

private:
    void updateSwatch();

    QColor m_color;
};

#endif // COLORBUTTON_H