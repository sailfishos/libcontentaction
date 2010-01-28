#include "contentaction.h"
#include <QCoreApplication>
#include <QTimer>
#include <QObject>
#include <QDebug>

class Dummy : public QObject
{
    Q_OBJECT
public slots:
    void timeout()
    {
        static int n = 0;
        qDebug() << "Trying to trigger" << (n+1);
        // Try to trigger an action...
        ContentAction::Action action = ContentAction::Action::defaultAction("an.image");
        qDebug() << action.name();
        action.trigger();
        if (++n >= 4) QCoreApplication::exit();
    }
};


int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    // Every 3 seconds, try to invoke the default action for an uri.
    QTimer timer;
    timer.setSingleShot(false);
    timer.setInterval(3000);

    Dummy dummy;
    QObject::connect(&timer, SIGNAL(timeout()), &dummy, SLOT(timeout()));

    dummy.timeout();
    timer.start();

    return app.exec();
}

#include "servicelistener.moc"

