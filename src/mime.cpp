/*
 * Copyright (C) 2010 Nokia Corporation.
 *
 * Contact: Marius Vollmer <marius.vollmer@nokia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include "contentaction.h"
#include "internal.h"

#include <stdlib.h>

#include <gio/gdesktopappinfo.h>
#include <gio/gio.h>
#include <glib.h>

#include <QStringList>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QFile>
#include <QHash>
#include <QDBusInterface>
#include <QDBusPendingCall>

namespace ContentAction {

using namespace ContentAction::Internal;

enum LaunchType {
    DontLaunch     = 0, ///< We have no clue.
    ExecLaunch     = 1, ///< Use the Exec line from the .desktop file
    DuiLaunch      = 2, ///< Use the Dui interface, and launch()
    MimeOpenLaunch = 3, ///< Legacy, do mime_open
};

/// Returns the content type of the given file, or an empty string if it cannot
/// be retrieved.
static QString contentTypeForFile(const QUrl& fileUri)
{
    g_type_init();
    QByteArray filename = fileUri.toEncoded();
    GFile *file = g_file_new_for_uri(filename.constData());

    GError *error = 0;
    GFileInfo *fileInfo;
    fileInfo = g_file_query_info(file,
                                 G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE,
                                 G_FILE_QUERY_INFO_NONE,
                                 NULL,
                                 &error);
    if (error != 0) {
        g_error_free(error);
        g_object_unref(file);
        return QString();
    }
    QString ret = QString::fromAscii(g_file_info_get_content_type(fileInfo));
    g_object_unref(fileInfo);
    g_object_unref(file);
    return ret;
}

// Returns the XDG data directories.
static const QStringList& xdgDataDirs()
{
    static bool dirsSet = false;
    static QStringList dirs;

    if (dirsSet)
        return dirs;

    dirsSet = true;
    const char *d;
    d = getenv("XDG_DATA_HOME");
    if (d)
        dirs.append(QString::fromLocal8Bit(d));
    else
        dirs.append(QDir::homePath() + "/.local/share");
    d = getenv("XDG_DATA_DIRS");
    if (!d)
        d = "/usr/local/share:/usr/share";
    dirs.append(QString::fromLocal8Bit(d).split(":", QString::SkipEmptyParts));
    return dirs;
}

// Reads "Key=Value" formatted lines from the file, and updates dict with
// them.
static void readKeyValues(QFile& file, QHash<QString, QString>& dict)
{
    if (!file.open(QIODevice::ReadOnly)) {
        LCA_WARNING << "cannot read" << file.fileName();
        return;
    }
    QTextStream stream(&file);
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        if (line.isNull())
            break;
        if (line.isEmpty() || line[0] == '[' || line[0] == '#')
            continue;
        QStringList keyAndValue = line.split('=', QString::SkipEmptyParts);
        if (keyAndValue.size() < 2)
            continue;
        dict.insert(keyAndValue[0], keyAndValue[1]);
    }
    file.close();
}

// Determines how to launch the application that the given .desktop file
// represents.  Returns the LaunchType and fills service, interface and method
// if applicable.
static LaunchType launchInfo(const QString& desktopFile,
                             QString& service,
                             QString& iface, QString& method)
{
    QFile file(desktopFile);
    if (!file.exists())
        return DontLaunch;
    QHash<QString, QString> appInfo;
    readKeyValues(file, appInfo);
    if (appInfo.contains("X-Maemo-Service")) {
        service = appInfo["X-Maemo-Service"];
        iface = "com.nokia.DuiApplicationIf";
        method = "launch";
        return DuiLaunch;
    }
    if (appInfo.contains("X-Osso-Service")) {
        service = appInfo["X-Osso-Service"];
        iface = service;
        method = "mime_open";
        return MimeOpenLaunch;
    }
    return ExecLaunch;
}

// Searches XDG dirs for the .desktop file with the given id
// ("something.desktop"), and returns the first hit.
static QString findDesktopFile(const QString& id)
{
    QStringList dirs = xdgDataDirs();
    for (int i = dirs.size()-1; i >= 0; --i) {
        QFile f(dirs[i] + "/applications/" + id);
        if (f.exists())
            return f.fileName();
    }
    return QString();
}

// Returns the default application for handling the given \a contentType. The
// default application is read from the defaults.list. If there is no default
// application, returns an empty string.
static QString defaultAppForContentType(const QString& contentType)
{
    static bool read = false;
    static QHash<QString, QString> defaultApps;

    if (!read) {
        QStringList dirs = xdgDataDirs();
        for (int i = dirs.size()-1; i >= 0; --i) {
            QFile f(dirs[i] + "/applications/defaults.list");
            if (!f.exists())
                continue;
            readKeyValues(f, defaultApps);
        }
    }
    read = true;
    return defaultApps.value(contentType, QString());
}

/// Returns the applications which handle the given \a contentType. The
/// applications are read from the mimeinfo.cache. The file is searched in the
/// default locations. The returned list will contain elements of the form
/// "appname.desktop".
static QStringList appsForContentType(const QString& contentType)
{
    static bool read = false;
    static QHash<QString, QString> mimeApps;

    if (!read) {
        QStringList dirs = xdgDataDirs();
        for (int i = dirs.size()-1; i >= 0; --i) {
            QFile f(dirs[i] + "/applications/mimeinfo.cache");
            if (!f.exists())
                continue;
            readKeyValues(f, mimeApps);
        }
    }

    read = true;
    QStringList ret;
    if (mimeApps.contains(contentType))
        ret = mimeApps[contentType].split(";", QString::SkipEmptyParts);

    // Get the default app handling this content type, insert it to the front
    // of the list
    QString defaultApp = defaultAppForContentType(contentType);
    if (!defaultApp.isEmpty()) {
        ret.removeAll(defaultApp);
        ret.prepend(defaultApp);
    }
    return ret;
}

struct MimePrivate: public Action::DefaultPrivate {
    MimePrivate(const QString& desktopFileName, const QList<QUrl>& fileUris);
    MimePrivate(const MimePrivate& other);
    virtual ~MimePrivate();
    virtual bool isValid() const;
    virtual QString name() const;
    virtual void trigger() const;
    virtual MimePrivate* clone() const;

    QString desktopFileName;
    QStringList fileNames;

    GAppInfo *appInfo;
    LaunchType kind;
    QString service;
    QString iface;
    QString method;
};

MimePrivate::MimePrivate(const QString& desktopFileName, const QList<QUrl>& fileUris) :
    desktopFileName(desktopFileName)
{
    foreach (const QUrl& uri, fileUris)
        fileNames << uri.toEncoded();
    kind = launchInfo(desktopFileName, service, iface, method);
    appInfo = G_APP_INFO(g_desktop_app_info_new_from_filename(
                             desktopFileName.toLocal8Bit().constData()));
}

MimePrivate::MimePrivate(const MimePrivate& other) :
    desktopFileName(other.desktopFileName),
    fileNames(other.fileNames),
    appInfo(g_app_info_dup(other.appInfo)),
    kind(other.kind),
    service(other.service),
    iface(other.iface),
    method(other.method)
{ }

MimePrivate::~MimePrivate()
{
    g_object_unref(appInfo);
}

bool MimePrivate::isValid() const
{
    return kind != DontLaunch;
}

QString MimePrivate::name() const
{
    const char *appName = g_app_info_get_name(appInfo);
    return QString::fromUtf8(appName);
}

/// Launches the application, either via D-Bus or with GIO.
void MimePrivate::trigger() const
{
    switch (kind) {
    case MimeOpenLaunch: {
        QVariantList vargs;
        foreach (const QString& file, fileNames)
            vargs << file;
        QDBusInterface launcher(service, "/", iface);
        launcher.callWithArgumentList(QDBus::NoBlock, method, vargs);
        break;
    }
    case DuiLaunch: {
        QDBusInterface launcher(service, "/", iface);
        launcher.asyncCall(method, fileNames);
        break;
    }
    case ExecLaunch: {
        GError *error = 0;
        GList *uris = NULL;

        foreach (const QString& file, fileNames)
            uris = g_list_append(uris, g_strdup(file.toAscii().constData()));

        g_app_info_launch_uris(appInfo, uris, NULL, &error);
        if (error != NULL) {
            LCA_WARNING << "cannot trigger: " << error->message;
            g_error_free(error);
        }
        g_list_foreach(uris, (GFunc)g_free, NULL);
        g_list_free(uris);
        break;
    }
    default: break;
    }
}

MimePrivate *MimePrivate::clone() const
{
    return new MimePrivate(*this);
}

/// Returns the default action for a given \a file, based on its content type.
Action Action::defaultActionForFile(const QUrl& fileUri)
{
    QString contentType = contentTypeForFile(fileUri);
    if (contentType.isEmpty())
        return Action();
    // We treat .desktop files specially: the default action (the only
    // actually) is to launch the application it describes.
    if (contentType == "application/x-desktop")
        return Action(new MimePrivate(fileUri.toLocalFile(), QList<QUrl>()));
    QString appid = defaultAppForContentType(contentType);
    if (appid.isEmpty())
        return Action();
    return Action(new MimePrivate(findDesktopFile(appid),
                                  QList<QUrl>() << fileUri));
}

/// Returns the set of applicable actions for a given \a file, based on its
/// content type.
QList<Action> Action::actionsForFile(const QUrl& fileUri)
{
    QList<Action> result;

    QString contentType = contentTypeForFile(fileUri);
    if (contentType.isEmpty())
        return result;
    if (contentType == "application/x-desktop")
        return result << Action(new MimePrivate(fileUri.toLocalFile(),
                                                QList<QUrl>()));

    QStringList appIds = appsForContentType(contentType);
    foreach (const QString& id, appIds) {
        result << Action(new MimePrivate(findDesktopFile(id),
                                         QList<QUrl>() << fileUri));
    }
    return result;
}

} // end namespace
