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
    virtual void trigger() const;
};

struct DefaultPrivate : public ActionPrivate
{
    DefaultPrivate(MDesktopEntry* desktopEntry, const QStringList& params,
        bool valid = true);
    virtual ~DefaultPrivate();
    virtual bool isValid() const;
    virtual QString name() const;
    virtual QString localizedName() const;
    virtual QString icon() const;
    MDesktopEntry* desktopEntry;
    QStringList params;
    bool valid;
};

struct ServiceFwPrivate : public DefaultPrivate {
    ServiceFwPrivate(MDesktopEntry* desktopEntry, const QStringList& params);
    virtual void trigger() const;

    QString serviceFwMethod;
};

struct DBusPrivate : public DefaultPrivate {
    DBusPrivate(MDesktopEntry* desktopEntry, const QStringList& params);
    virtual void trigger() const;

    QString busName;
    QString objectPath;
    QString iface;
    QString method;
    bool varArgs;
};

struct ExecPrivate : public DefaultPrivate {
    ExecPrivate(MDesktopEntry* desktopEntry, const QStringList& params);
    virtual ~ExecPrivate();
    virtual void trigger() const;

    GAppInfo *appInfo;
};

Action createAction(const QString& desktopFileId, const QStringList& params);

// our pseudo mimetype classes
extern const QString OntologyMimeClass;
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
LCA_EXPORT QStringList appsForContentType(const QString& contentType);
LCA_EXPORT QString defaultAppForContentType(const QString& contentType);
QString findDesktopFile(const QString& id);

LCA_EXPORT QString mimeForScheme(const QString& uri);
LCA_EXPORT QString mimeForFile(const QUrl& fileUri);
LCA_EXPORT QStringList mimeForTrackerObject(const QString& uri);

const QHash<QString, QStringList>& mimeApps();
const QHash<QString, QString>& highlighterConfig();
const QHash<QString, QString>& trackerConditions();

} // end namespace Internal
} // end namespace ContentAction
#endif
