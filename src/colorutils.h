#ifndef COLORUTILS_H
#define COLORUTILS_H

#include <QWidget>
#include <QColor>

void setFrameColorBasedOnWindow(QWidget *window, QWidget *frame);
QColor adjustColor(const QColor &color, double factor);
bool isDarkMode(const QColor &color);

#endif // COLORUTILS_H
