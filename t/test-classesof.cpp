#include "contentaction.h"
#include <QCoreApplication>
#include <QDebug>

int main(int argc, char **argv)
{
    //qDebug() << ContentAction::classesOf("urn:uuid:1930642225");
    QCoreApplication app(argc, argv);
    QList<ContentAction> acts = ContentAction::actions("an.image");
    foreach (const ContentAction& act, acts) {
        act.trigger();
    }
    return app.exec();
}
