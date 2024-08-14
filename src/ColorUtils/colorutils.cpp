#include "colorutils.h"
#include <QBrush>
#include <QColor>
#include <QPalette>
#include "qmainwindow.h"

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

void setWidgetBgColorBasedOnWindow(QWidget *window, QWidget *widget, double darkMultiplyer, double lightMultiplyer)
{
    QColor mainBgColor = window->palette().color(QPalette::Window);

    QColor widgetBgColor;
    if (isDarkMode(mainBgColor)) {
        widgetBgColor = adjustColor(mainBgColor, darkMultiplyer);
    } else {
        widgetBgColor = adjustColor(mainBgColor, lightMultiplyer);
    }

    QPalette palette = widget->palette();
    palette.setBrush(QPalette::Window, QBrush(widgetBgColor));
    widget->setAutoFillBackground(true);
    widget->setPalette(palette);
}

void setMainWindowBgColorBasedOnWindow(QMainWindow *mainWindow, double darkMultiplyer, double lightMultiplyer)
{
    QColor mainBgColor = mainWindow->palette().color(QPalette::Window);

    QColor newBgColor;
    if (isDarkMode(mainBgColor)) {
        newBgColor = adjustColor(mainBgColor, darkMultiplyer);
    } else {
        newBgColor = adjustColor(mainBgColor, lightMultiplyer);
    }

    QPalette palette = mainWindow->palette();
    palette.setBrush(QPalette::Window, QBrush(newBgColor));
    mainWindow->setAutoFillBackground(true);
    mainWindow->setPalette(palette);
}
