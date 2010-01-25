#include "servicelistenerdummy.h"

#include <QCoreApplication>
#include <QTimer>

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    // Every 10 seconds, try to invoke the default action for an uri.
    QTimer timer;
    timer.setSingleShot(false);
    timer.setInterval(10000);

    Dummy dummy;
    QObject::connect(&timer, SIGNAL(timeout()), &dummy, SLOT(timeout()));

    dummy.timeout();
    timer.start();

    return app.exec();
}
