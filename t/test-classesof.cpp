#include "contentaction.h"
#include <QDebug>

int main()
{
    qDebug() << ContentAction::classesOf("urn:uuid:1930642225");
    return 0;
}
