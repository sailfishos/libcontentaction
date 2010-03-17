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

class DuiDesktopEntry;

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
    DefaultPrivate(DuiDesktopEntry* desktopEntry, const QStringList& params);
    virtual ~DefaultPrivate();
    virtual bool isValid() const;
    virtual QString name() const;
    virtual QString localizedName() const;
    virtual QString icon() const;
    DuiDesktopEntry* desktopEntry;
    QStringList params;
};

struct ServiceFwPrivate : public DefaultPrivate {
    ServiceFwPrivate(DuiDesktopEntry* desktopEntry, const QStringList& params);
    virtual void trigger() const;

    QString serviceFwMethod;
};

struct DBusPrivate : public DefaultPrivate {
    DBusPrivate(DuiDesktopEntry* desktopEntry, const QStringList& params);
    virtual void trigger() const;

    QString busName;
    QString objectPath;
    QString iface;
    QString method;
    bool varArgs;
};

struct ExecPrivate : public DefaultPrivate {
    ExecPrivate(DuiDesktopEntry* desktopEntry, const QStringList& params);
    virtual ~ExecPrivate();
    virtual void trigger() const;

    GAppInfo *appInfo;
};

Action createAction(const QString& desktopFileId, const QStringList& params);

namespace Internal {

extern const QString XMaemoServiceKey;
extern const QString XOssoServiceKey;
extern const QString XMaemoMethodKey;
extern const QString XMaemoObjectPathKey;
extern const QString XMaemoFixedArgsKey;
extern const QString ExecKey;

QList<Action> actionsForUri(const QString& uri, const QString& mimeType);
QStringList appsForContentType(const QString& contentType);
QString defaultAppForContentType(const QString& contentType);
QString findDesktopFile(const QString& id);
QString contentTypeForFile(const QUrl& fileUri);
QStringList mimeTypesForUri(const QString& uri);

const QHash<QString, QStringList>& mimeApps();
const QHash<QString, QString>& highlighterConfig();
const QHash<QString, QString>& trackerConditions();

} // end namespace Internal
} // end namespace ContentAction
#endif
