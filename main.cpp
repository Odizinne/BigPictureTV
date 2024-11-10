#include <QApplication>
#include <QDebug>
#include <QLocale>
#include <QString>
#include <QTranslator>
#include <QProcess>
#include "BigPictureTV.h"
#include "Utils.h"

bool isAnotherInstanceRunning(const QString& processName)
{
    QProcess process;
    process.start("tasklist", QStringList() << "/FI" << QString("IMAGENAME eq %1").arg(processName));
    process.waitForFinished();
    QString output = process.readAllStandardOutput();
    int count = output.count(processName, Qt::CaseInsensitive);

    return count > 1;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false);
    if (Utils::isWindows10()) {
        a.setStyle("fusion");
    }

    const QString processName = "BigPictureTV.exe";
    if (isAnotherInstanceRunning(processName)) {
        qDebug() << "Another instance is already running. Exiting...";
        return 0;
    }

    QLocale locale;
    QString languageCode = locale.name().section('_', 0, 0);
    QTranslator translator;
    if (translator.load(":/translations/BigPictureTV_" + languageCode + ".qm")) {
        a.installTranslator(&translator);
    }

    BigPictureTV w;

    return a.exec();
}
