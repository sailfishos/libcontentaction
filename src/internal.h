#ifndef INTERNAL_H
#define INTERNAL_H

#include <QHash>
#include <QList>
#include <QPair>

namespace ContentAction {

#define LCA_WARNING qWarning() << "libcontentaction:"

typedef QHash<QString, QList<QPair<int, QString> > > Associations;
typedef QHash<QString, QStringList> HighlighterMap;

const Associations& actionsForClasses();
QStringList classesOf(const QString& uri);
QList<QPair<int, QString> > actionsForClass(const QString& klass);
QString defaultActionForClass(const QString& klass);
bool setDefaultAction(const QString& klass, const QString& action);
QString defaultActionFromGConf(const QString& klass);
QString defaultActionForClasses(const QStringList& classes);

const HighlighterMap& highlighterConfig();
}

#endif
