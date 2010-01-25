#include "contentaction.h"
#include <QObject>
#include <QDebug>

class Dummy : public QObject
{
    Q_OBJECT
public slots:
    void timeout()
    {
        qDebug() << "Trying to trigger";
        // Try to trigger an action...
        ContentAction::Action action = ContentAction::Action::defaultAction("an.image");
        qDebug() << action.name();
        action.trigger();
    }
};
