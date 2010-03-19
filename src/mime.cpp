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
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>
#include <QFile>
#include <QHash>
#include <QDBusInterface>
#include <QDBusPendingCall>

#include <DuiDesktopEntry>

namespace ContentAction {

using namespace ContentAction::Internal;

static const QString DesktopFileMimeType("application/x-desktop");
static const QString UriSchemeMimeClass("x-maemo-urischeme/");

/// Returns the content type of the given file, or an empty string if it cannot
/// be retrieved.
QString Internal::contentTypeForFile(const QUrl& fileUri)
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
        int eq = line.indexOf('=');
        if (eq < 0)
            continue;
        dict.insert(line.left(eq).trimmed(), line.mid(eq + 1).trimmed());
    }
    file.close();
}

// Searches XDG dirs for the .desktop file with the given id
// ("something.desktop"), and returns the first hit.
QString Internal::findDesktopFile(const QString& id)
{
    QStringList dirs = xdgDataDirs();
    for (int i = 0; i < dirs.size(); ++i) {
        QFile f(dirs[i] + "/applications/" + id);
        if (f.exists())
            return f.fileName();
    }
    return QString();
}

// Returns the default application for handling the given \a contentType. The
// default application is read from the defaults.list. If there is no default
// application, returns an empty string.
QString Internal::defaultAppForContentType(const QString& contentType)
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

// Reads the mimeinfo.cache files and returns the mapping from mime types to
// desktop entries.
const QHash<QString, QStringList>& Internal::mimeApps()
{
    static bool read = false;
    static QHash<QString, QStringList> mimecache;

    if (read)
        return mimecache;

    QStringList dirs = xdgDataDirs();
    QHash<QString, QString> temp;
    for (int i = dirs.size()-1; i >= 0; --i) {
        QFile f(dirs[i] + "/applications/mimeinfo.cache");
        if (!f.exists())
            continue;
        readKeyValues(f, temp);
    }
    QHashIterator<QString, QString> it(temp);
    while (it.hasNext()) {
        it.next();
        mimecache.insert(it.key(), it.value().split(";", QString::SkipEmptyParts));
    }
    read = true;
    return mimecache;
}

/// Returns the applications which handle the given \a contentType. The
/// applications are read from the mimeinfo.cache. The file is searched in the
/// default locations. The returned list will contain elements of the form
/// "appname.desktop".
QStringList Internal::appsForContentType(const QString& contentType)
{
    QStringList ret;

    if (mimeApps().contains(contentType))
        ret = mimeApps()[contentType];

    // Get the default app handling this content type, insert it to the front
    // of the list
    QString defaultApp = defaultAppForContentType(contentType);
    if (!defaultApp.isEmpty()) {
        ret.removeAll(defaultApp);
        ret.prepend(defaultApp);
    }
    return ret;
}

/// Returns the default action for a given \a fileUri, based on its content
/// type.
Action Action::defaultActionForFile(const QUrl& fileUri)
{
    QString contentType = contentTypeForFile(fileUri);
    if (contentType.isEmpty())
        return Action();
    return defaultActionForFile(fileUri, contentType);
}

/// Returns the default actions for a given \a fileUri, assuming its mime type
/// is \a mimeType. This function can be used even when \a fileUri doesn't
/// exist yet but will be created before trigger() is called, or if you
/// already know the mime type. Note: if the file is a .desktop file, it must
/// exist when this function is called.
Action Action::defaultActionForFile(const QUrl& fileUri, const QString& mimeType)
{
    // We treat .desktop files specially: the default action (the only
    // actually) is to launch the application it describes.
    if (mimeType == DesktopFileMimeType)
        return createAction(fileUri.toLocalFile(), QStringList());
    QString appid = defaultAppForContentType(mimeType);
    if (!appid.isEmpty())
        return createAction(findDesktopFile(appid),
                        QStringList() << fileUri.toEncoded());
    // Fall back to one of the existing actions (if there are some)
    QList<Action> acts = actionsForUri(fileUri.toEncoded(), mimeType);
    if (acts.size() >= 1)
        return acts[0];
    return Action();
}

QList<Action> Internal::actionsForUri(const QString& uri, const QString& mimeType)
{
    QList<Action> result;

    if (mimeType == DesktopFileMimeType)
        return result << createAction(uri, QStringList());

    QStringList appIds = appsForContentType(mimeType);
    foreach (const QString& id, appIds) {
        result << createAction(findDesktopFile(id),
                               QStringList() << uri);
    }
    return result;
}

/// Returns the set of applicable actions for a given \a fileUri, based on its
/// content type.
QList<Action> Action::actionsForFile(const QUrl& fileUri)
{
    QString contentType = contentTypeForFile(fileUri);
    return actionsForFile(fileUri, contentType);
}

/// Returns the set of applicable actions for a given \a fileUri, assuming its
/// content type is \a mimeType.  This function can be used even when \a
/// fileUri doesn't exist but will be created before trigger() is called, or
/// if you already know the mime type.  Note: if the file is a .desktop file,
/// it must exist when this function is called.
QList<Action> Action::actionsForFile(const QUrl& fileUri, const QString& mimeType)
{
    if (mimeType == DesktopFileMimeType)
        return actionsForUri(fileUri.toLocalFile(), mimeType);

    return actionsForUri(fileUri.toEncoded(), mimeType);
}

/// Returns the default action for handling the scheme of the passed \a uri.
/// \sa actionsForScheme().
Action Action::defaultActionForScheme(const QString& uri)
{
    int n = uri.indexOf(':');
    if (n < 0)
        return Action();
    QString mime(UriSchemeMimeClass + uri.left(n));
    QString defApp = defaultAppForContentType(mime);
    if (defApp.isEmpty())
        return Action();
    return createAction(findDesktopFile(defApp), QStringList() << uri);
}

/// Returns all actions handling the scheme of the given \a uri.  The uri
/// scheme is mapped to mime types by prefixing it with \c
/// "x-maemo-urischeme/".  For example an email client may declare to handle
/// the \c "x-maemo-urischeme/mailto" mimetype and a browser then just
/// triggers the returned Action to activate a \c mailto: link.
QList<Action> Action::actionsForScheme(const QString& uri)
{
    QList<Action> result;
    int n = uri.indexOf(':');
    if (n < 0)
        return result;
    QString mime(UriSchemeMimeClass + uri.left(n));
    foreach (const QString& app, appsForContentType(mime)) {
        result << createAction(findDesktopFile(app), QStringList() << uri);
    }
    return result;
}

} // end namespace
