#include "aboutwindow.h"
#include "ui_aboutwindow.h"
#include "steamwindowmanager.h"
#include "colorutils.h"

AboutWindow::AboutWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AboutWindow)
{
    ui->setupUi(this);
    ui->detectedSteamLanguage->setText(getSteamLanguage());
    ui->targetWindowTitle->setText(getBigPictureWindowTitle());
    ui->repository->setText("<a href=\"https://github.com/odizinne/bigpicturetv/\">github.com/Odizinne/BigPictureTV</a>");
    ui->repository->setTextFormat(Qt::RichText);
    ui->repository->setTextInteractionFlags(Qt::TextBrowserInteraction);
    ui->repository->setOpenExternalLinks(true);
    ui->commitID->setText(GIT_COMMIT_ID);
    ui->commitDate->setText(GIT_COMMIT_DATE);
    ui->branch->setText(GIT_BRANCH);
}

AboutWindow::~AboutWindow()
{
    delete ui;
}
