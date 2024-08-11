#include "colorutils.h"
#include <QPalette>
#include <QBrush>

void setFrameColorBasedOnWindow(QWidget *window, QWidget *frame)
{
    QColor mainBgColor = window->palette().color(QPalette::Window);

    QColor frameBgColor;
    if (isDarkMode(mainBgColor)) {
        frameBgColor = adjustColor(mainBgColor, 1.5);
    } else {
        frameBgColor = adjustColor(mainBgColor, 0.95);
    }

    QPalette palette = frame->palette();
    palette.setBrush(QPalette::Window, QBrush(frameBgColor));
    frame->setAutoFillBackground(true);
    frame->setPalette(palette);
}

QColor adjustColor(const QColor &color, double factor)
{
    int r = qBound(0, static_cast<int>(color.red() * factor), 255);
    int g = qBound(0, static_cast<int>(color.green() * factor), 255);
    int b = qBound(0, static_cast<int>(color.blue() * factor), 255);
    return QColor(r, g, b, color.alpha());
}

bool isDarkMode(const QColor &color)
{
    int r = color.red();
    int g = color.green();
    int b = color.blue();
    int brightness = (r + g + b) / 3;
    return brightness < 127;
}
