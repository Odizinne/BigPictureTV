#include <QApplication>
#include <QDebug>
#include <QLocale>
#include <QMessageBox>
#include <QSharedMemory>
#include <QString>
#include <QTranslator>
#include "bigpicturetv.h"

int main(int argc, char *argv[])
{
    QSharedMemory sharedMemory("BigPictureTVUniqueIdentifier");

    if (sharedMemory.attach()) {
        return 0;
    }

    if (!sharedMemory.create(1)) {
        qDebug() << "Unable to create shared memory segment.";
        return 1;
    }

    QApplication a(argc, argv);

    QLocale locale;
    QString languageCode = locale.name().section('_', 0, 0);
    QTranslator translator;
    if (translator.load(":/translations/BigPictureTV_" + languageCode + ".qm")) {
        a.installTranslator(&translator);
    }
    a.setStyle("fusion");
    a.setQuitOnLastWindowClosed(false);
    BigPictureTV w;
    QObject::connect(&a, &QApplication::aboutToQuit, [&sharedMemory]() { sharedMemory.detach(); });

    return a.exec();
}
