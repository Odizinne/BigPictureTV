#include "bigpicturetv.h"
#include <QApplication>
#include <QSharedMemory>
#include <QMessageBox>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle("fusion");

    // Create a QSharedMemory object
    QSharedMemory sharedMemory("BigPictureTVUniqueIdentifier");

    // Try to attach to the shared memory segment
    if (sharedMemory.attach()) {
        // If attach is successful, it means another instance is running
        return 0; // Exit the current instance
    }

    // Try to create the shared memory segment
    if (!sharedMemory.create(1)) {
        qDebug() << "Unable to create shared memory segment.";
        return 1; // Exit with an error code
    }

    BigPictureTV w;

    // Release the shared memory when the application exits
    QObject::connect(&a, &QApplication::aboutToQuit, [&sharedMemory]() {
        sharedMemory.detach();
    });

    return a.exec();
}
