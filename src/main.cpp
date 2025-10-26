#include <QApplication>
#include <QQmlApplicationEngine>
#include <QDebug>
#include <QProcess>
#include "appconfiguration.h"
#include "appbridge.h"
#include "utils.h"

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
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    app.setOrganizationName("Odizinne");
    app.setApplicationName("BigPictureTV");

    if (Utils::isWindows10()) {
        app.setStyle("fusion");
    }

    const QString processName = "BigPictureTV.exe";
    if (isAnotherInstanceRunning(processName)) {
        qDebug() << "Another instance is already running. Exiting...";
        return 0;
    }

    QQmlApplicationEngine engine;

    // Create singletons before loading QML
    // This ensures proper initialization order
    AppConfiguration::create(&engine, nullptr);
    AppBridge::create(&engine, nullptr);

    engine.loadFromModule("Odizinne.BigPictureTV", "Main");

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
