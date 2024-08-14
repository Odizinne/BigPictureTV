#ifndef COLORUTILS_H
#define COLORUTILS_H

#include <QWidget>
#include "qmainwindow.h"

void setWidgetBgColorBasedOnWindow(QWidget *window, QWidget *frame, double darkMultiplyer, double lightMultiplyer);
void setMainWindowBgColorBasedOnWindow(QMainWindow *mainWindow, double darkMultiplyer, double lightMultiplyer);

#endif // COLORUTILS_H
