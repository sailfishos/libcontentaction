#ifndef INTERNAL_H
#define INTERNAL_H

namespace ContentAction
{
QStringList classesOf(const QString& uri);
QList<QPair<int, QString> > actionsForClass(const QString& klass);
QString defaultActionForClass(const QString& klass);
bool setDefaultAction(const QString& klass, const QString& action);
QString defaultAction(const QString& klass);
QString defaultActionForClasses(const QStringList& classes);
}

#endif
