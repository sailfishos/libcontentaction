#ifndef INTERNAL_H
#define INTERNAL_H

#include "contentaction.h"
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

struct MimePrivate: Action::DefaultPrivate {
    MimePrivate(const QUrl& fileUri, struct _GAppInfo* app);
    virtual ~MimePrivate();
    virtual bool isValid() const;
    virtual QString name() const;
    virtual void trigger() const;
    virtual DefaultPrivate *clone() const;

    QUrl fileUri;
    struct _GAppInfo* appInfo;
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

Action mimeAction(const QUrl& fileUri,
                  struct _GAppInfo* appInfo);

// regexp -> actions
typedef QHash<QString, QStringList> HighlighterMap;

const HighlighterMap& highlighterConfig();

char* contentTypeForFile(const char* fileUri);
}
}
#endif
