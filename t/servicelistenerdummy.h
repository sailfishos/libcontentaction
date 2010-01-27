#include "contentaction.h"
#include <QObject>
#include <QCoreApplication>
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
