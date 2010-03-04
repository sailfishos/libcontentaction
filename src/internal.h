#ifndef INTERNAL_H
#define INTERNAL_H

#include "contentaction.h"
#include "service.h"

#include <QHash>
#include <QList>
#include <QPair>
#include <QStringList>

#define LCA_WARNING qWarning() << "libcontentaction:"

namespace ContentAction {

struct Action::DefaultPrivate
{
    virtual ~DefaultPrivate();
    virtual void setAsDefault();
    virtual bool isDefault() const;
    virtual bool canBeDefault() const;
    virtual bool isValid() const;
    virtual QString name() const;
    virtual QString localizedName() const;
    virtual void trigger() const;
    virtual DefaultPrivate *clone() const;
};

namespace Internal {

typedef QHash<QString, QList<QPair<int, QString> > > Associations;

const Associations& actionsForClasses();
QStringList classesOf(const QString& uri);
QList<QPair<int, QString> > actionsForClass(const QString& klass);
QString defaultActionForClass(const QString& klass);
bool setDefaultAction(const QString& klass, const QString& action);
QString defaultActionFromGConf(const QString& klass);
QString defaultActionForClasses(const QStringList& classes);

Action trackerAction(const QStringList& uris,
                     const QStringList& classes,
                     const QString& action);

Action highlightAction(const QString& text,
                       const QString& action);

void initializeLocales();

// regexp -> actions
typedef QHash<QString, QStringList> HighlighterMap;

const HighlighterMap& highlighterConfig();

const QStringList translationsConfig();

} // end namespace Internal
} // end namespace ContentAction
#endif
