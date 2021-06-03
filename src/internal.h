#ifndef INTERNAL_H
#define INTERNAL_H

#include "contentaction.h"
#include "service.h"

#include <QHash>
#include <QList>
#include <QPair>
#include <QStringList>
#include <QDebug>

#include <gio/gdesktopappinfo.h>

#define LCA_WARNING qWarning() << "libcontentaction:"

class MDesktopEntry;

namespace ContentAction {

struct ActionPrivate
{
    virtual ~ActionPrivate();
    virtual bool isValid() const;
    virtual QString name() const;
    virtual QString localizedName() const;
    virtual QString icon() const;
    virtual void trigger(bool wait) const;
};

struct DefaultPrivate : public ActionPrivate
{
    DefaultPrivate(QSharedPointer<MDesktopEntry> desktopEntry,
                   const QStringList& params,
        bool valid = true);
    virtual ~DefaultPrivate();
    virtual bool isValid() const;
    virtual QString name() const;
    virtual QString localizedName() const;
    virtual QString icon() const;
    QSharedPointer<MDesktopEntry> desktopEntry;
    QStringList params;
    bool valid;
};

struct ServiceFwPrivate : public DefaultPrivate {
    ServiceFwPrivate(QSharedPointer<MDesktopEntry> desktopEntry,
                     const QStringList& params);
    virtual void trigger(bool) const;

    QString serviceFwMethod;
};

struct DBusPrivate : public DefaultPrivate {
    DBusPrivate(QSharedPointer<MDesktopEntry> desktopEntry,
                const QStringList& params);
    virtual void trigger(bool) const;

    QString busName;
    QString objectPath;
    QString iface;
    QString method;
    bool varArgs;
};

struct ExecPrivate : public DefaultPrivate {
    ExecPrivate(QSharedPointer<MDesktopEntry> desktopEntry,
                const QStringList& params);
    virtual ~ExecPrivate();
    virtual void trigger(bool) const;

    GDesktopAppInfo *appInfo;
};

Action createAction(const QString& desktopFilePath,
                    const QStringList& params);
Action createAction(QSharedPointer<MDesktopEntry> desktopEntry,
                    const QStringList& params);

// our pseudo mimetype classes
extern const QString HighlighterMimeClass;
extern const QString UriSchemeMimeClass;

namespace Internal {

// custom .desktop file keys
extern const QString XMaemoServiceKey;
extern const QString XOssoServiceKey;
extern const QString XMaemoMethodKey;
extern const QString XMaemoObjectPathKey;
extern const QString XMaemoFixedArgsKey;
extern const QString ExecKey;

QList<Action> actionsForUri(const QString& uri, const QString& mimeType);
QList<Action> actionsForUris(const QStringList& uri, const QString& mimeType);
LCA_EXPORT QStringList appsForContentType(const QString& contentType);
LCA_EXPORT QString defaultAppForContentType(const QString& contentType);
QString findDesktopFile(const QString& id);

LCA_EXPORT QString mimeForScheme(const QString& uri);
LCA_EXPORT QString mimeForFile(const QUrl& fileUri);
LCA_EXPORT QStringList mimeForString(const QString& param);

const QHash<QString, QStringList>& mimeApps();
const QList<QPair<QString, QRegExp> >& highlighterConfig();

QString bindParams(const QString &str);

QRegExp masterRegexp();

} // end namespace Internal
} // end namespace ContentAction
#endif
