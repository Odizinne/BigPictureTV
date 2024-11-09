#include <QApplication>
#include <QDebug>
#include <QLocale>
#include <QMessageBox>
#include <QLocalServer>
#include <QLocalSocket>
#include <QString>
#include <QTranslator>
#include "BigPictureTV.h"
#include "Utils.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString serverName = "BigPictureTVUniqueIdentifier";
    QLocalServer localServer;
    QLocalSocket localSocket;

    localSocket.connectToServer(serverName);

    if (localSocket.waitForConnected(500)) {
        qDebug() << "Another instance is already running.";
        return 0;
    }

    if (!localServer.listen(serverName)) {
        qDebug() << "Unable to start the local server:" << localServer.errorString();
        return 1;
    }

    QLocale locale;
    QString languageCode = locale.name().section('_', 0, 0);
    QTranslator translator;

    if (translator.load(":/translations/BigPictureTV_" + languageCode + ".qm")) {
        a.installTranslator(&translator);
    }

    if (Utils::isWindows10()) {
        a.setStyle("fusion");
    }
    a.setQuitOnLastWindowClosed(false);

    BigPictureTV w;

    QObject::connect(&a, &QApplication::aboutToQuit, [&localServer]() {
        localServer.close();
    });

    return a.exec();
}
