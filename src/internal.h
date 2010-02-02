#ifndef INTERNAL_H
#define INTERNAL_H

#include "contentaction.h"
#include "contentaction_internal.h"
#include <QHash>
#include <QList>
#include <QPair>

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
    virtual void trigger() const;
    virtual DefaultPrivate *clone() const;
};

struct TrackerPrivate: public Action::DefaultPrivate
{
    TrackerPrivate(const QStringList& uris,
                   const QStringList& classes,
                   const QString& action);
    virtual ~TrackerPrivate();
    virtual void setAsDefault();
    virtual bool isDefault() const;
    virtual bool canBeDefault() const;
    virtual bool isValid() const;
    virtual QString name() const;
    virtual void trigger() const;
    virtual DefaultPrivate *clone() const;

    QStringList uris; ///< the target uri's of the action
    QStringList classes; ///< the classes of the uri's (if they are of the
                         ///< same type)
    QString action; ///< [service fw interface].[method]
};

struct HighlightPrivate: public Action::DefaultPrivate
{
    HighlightPrivate(const QString& match, const QString& action);
    virtual ~HighlightPrivate();
    virtual bool isValid() const;
    virtual QString name() const;
    virtual void trigger() const;
    virtual DefaultPrivate *clone() const;

    QString match;
    QString action;
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

}
}
#endif
