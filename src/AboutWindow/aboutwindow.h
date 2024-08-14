#ifndef ABOUTWINDOW_H
#define ABOUTWINDOW_H

#include <QDialog> // Change to QDialog

namespace Ui {
class AboutWindow;
}

class AboutWindow : public QDialog // Inherit from QDialog
{
    Q_OBJECT

public:
    explicit AboutWindow(QWidget *parent = nullptr);
    ~AboutWindow();

private:
    Ui::AboutWindow *ui;
};

#endif // ABOUTWINDOW_H
