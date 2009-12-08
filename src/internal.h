#ifndef INTERNAL_H
#define INTERNAL_H

namespace ContentAction
{
QStringList classesOf(const QString& uri);
QStringList actionsForClass(const QString& klass);
QString defaultActionForClass(const QString& klass);
}

#endif
