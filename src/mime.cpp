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
#include <stdio.h>

#include <gio/gdesktopappinfo.h>
#include <gio/gio.h>
#include <glib.h>

#include <QStringList>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QTextStream>
#include <QDebug>
#include <QFile>
#include <QHash>
#include <QDBusInterface>
#include <QDBusPendingCall>

#include <MDesktopEntry>

#include <sys/stat.h>

namespace { // Helper functions

bool readLastModifiedTime(const QString& filename, long& time)
{
    struct stat statData;
    if (stat(filename.toLatin1().constData(), &statData) == 0) {
        time = statData.st_mtim.tv_sec * 1000 + statData.st_mtim.tv_nsec / 1000000;
        return true;
    }
    return false;
}

// Reads "Key=Value" formatted lines from the file, and updates dict with
// them.
void readKeyValues(QFile& file, QHash<QString, QString>& dict)
{
    if (!file.open(QIODevice::ReadOnly)) {
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


QHash<QString, QString> readChangedKeyValueFiles(const QStringList& dirs,
                                                 const QStringList& suffixes)
{
    // Read each file if it has changed since the last time we read it.

    // Using the "last modified" time for this purpose might be stupid, but what
    // else can we do.  Using a QFileSystemWatcher (or a similar solution) in a
    // library without a LifeTimeManager object is a bad idea too.  Deleting the
    // QFileSystemWatcher at the right moment is tricky and cannot be enforced
    // by the library.

    // The resolution of the "last modified time" is one second.  This will be a
    // problem if somebody sets the mime default twice during the same second,
    // and somebody reads it between those two changes.  In this case, the
    // second change remains undetected by the reading process, until the file
    // is changed again.

    // The "last modified" times of the files this function has read.
    static QHash<QString, long> lastModified;

    QHash<QString, QString> temp;
    // Whether a file has changed so that we need to re-read all the overriding
    // files, too.
    bool rereadAllThatFollows = false;
    // Read the files in such a order that the first ones override the later
    // ones.
    for (int i = dirs.size()-1; i >= 0; --i) {
        for (int suffix = 0; suffix < suffixes.size(); suffix++) {
            QString filename = dirs[i] + suffixes[suffix];
            if (QFileInfo::exists(filename)) {
                long lm = 0;
                if (!readLastModifiedTime(filename, lm)) {
                    // Most probably the file doesn't exist.
                    continue;
                }
                if (!rereadAllThatFollows && lm == lastModified.value(filename)) {
                    continue;
                }
                QFile f(filename);
                readKeyValues(f, temp);
                lastModified[filename] = lm;
                rereadAllThatFollows = true;
                break;
            }
        }
    }
    return temp;
}

} // end of unnamed namespace

namespace ContentAction {

using namespace ContentAction::Internal;

const QString UriSchemeMimeClass("x-scheme-handler/");

static const QString DesktopFileMimeType("application/x-desktop");

static QString generalizeMimeType(const QString &mime)
{
    // No wildcards for our pseudo mimetypes.
    if (mime.startsWith(OntologyMimeClass) ||
        mime.startsWith(HighlighterMimeClass) ||
        mime.startsWith(UriSchemeMimeClass))
        return "";
    int n = mime.indexOf('/');
    if (n < 0)
        return mime;
    return QString(mime.left(n) + "/*");
}

/// Returns the content type of the given file, or an empty string if it cannot
/// be retrieved.
QString Internal::mimeForFile(const QUrl& uri)
{
    // assume "file" scheme if the uri had nothing
    QUrl fileUri(uri);
    if (fileUri.scheme().isEmpty())
        fileUri.setScheme("file");
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
        // Try again without GFileInfo, which might only work if the
        // file actually exists.
        char *type = g_content_type_guess (fileUri.toLocalFile().toLocal8Bit().constData(),
                                           NULL, 0, NULL);
        QString res;
        if (type) {
            gchar *temp = g_content_type_get_mime_type (type);
            res = temp;
            g_free (temp);
        }
        g_free (type);
        return res;
    }
    QString ret = QString::fromLatin1(g_file_info_get_content_type(fileInfo));
    g_object_unref(fileInfo);
    g_object_unref(file);
    return ret;
}

static QString xdgDataHome()
{
    const char *d;
    d = getenv("XDG_DATA_HOME");
    if (d)
        return QString::fromLocal8Bit(d);
    else
        return QDir::homePath() + "/.local/share";
}

static const QStringList& xdgDataDirs()
{
    static bool dirsSet = false;
    static QStringList dirs;

    if (dirsSet)
        return dirs;

    dirsSet = true;
    dirs.append(xdgDataHome());
    const char *d;
    d = getenv("XDG_DATA_DIRS");
    if (!d)
        d = "/usr/local/share:/usr/share";
    dirs.append(QString::fromLocal8Bit(d).split(":", QString::SkipEmptyParts));
    return dirs;
}

static const QString getMimeFile(QString rootPath)
{
    QString mimeFileDB = rootPath + "/applications/mimeapps.list";
    if (!QFileInfo::exists(mimeFileDB))
        mimeFileDB = rootPath + "/applications/defaults.list";
    return mimeFileDB;
}

static void writeDefaultsList(const QHash<QString, QString>& defaults)
{
    QString targetFileName = xdgDataHome() + "/applications/mimeapps.list";
    QFile outFile(targetFileName + ".temp");
    if (!outFile.open(QIODevice::WriteOnly)) {
        LCA_WARNING << "cannot write" << outFile.fileName();
        return;
    }
    QTextStream stream(&outFile);

    stream << "[Default Applications]" << endl;
    for (QHash<QString, QString>::ConstIterator it = defaults.constBegin();
         it != defaults.constEnd(); ++it) {
        stream << it.key() << "=" << it.value() << endl;
    }
    outFile.close();

    // Unfortunately, QFile doesn't provide an API to do this (renaming a file
    // and thus overwriting an existing file)
    ::rename((targetFileName + ".temp").toLatin1().constData(),
             targetFileName.toLatin1().constData());

}

/// Sets the \a action as a default application to the given \a mimeType.
void setMimeDefault(const QString& mimeType, const Action& action)
{
    setMimeDefault(mimeType, action.name());
}

/// Sets the \a app as a default application to the given \a mimeType. \a app
/// must be a application name, i.e., the base file name of the .desktop file,
/// without the .desktop extension.
void setMimeDefault(const QString& mimeType, const QString& app)
{
    QHash<QString, QString> defaults;
    // Read the contents of $XDG_DATA_HOME/applications/mimeapps.list (if
    // it exists)
    QFile file(getMimeFile(xdgDataHome()));
    readKeyValues(file, defaults);

    defaults[mimeType] = app + ".desktop";

    // Write back
    writeDefaultsList(defaults);
}

/// Removes the association between \a mimeType and a user-configured default
/// action.
void resetMimeDefault(const QString& mimeType)
{
    QHash<QString, QString> defaults;
    // Read the contents of $XDG_DATA_HOME/applications/mimeapps.list (if
    // it exists)
    QFile file(getMimeFile(xdgDataHome()));
    readKeyValues(file, defaults);

    defaults.remove(mimeType);

    // Write back
    writeDefaultsList(defaults);
}

// Searches XDG dirs for the .desktop file with the given id
// ("something.desktop"), and returns the first hit.
QString Internal::findDesktopFile(const QString& id)
{
    if (id.isEmpty())
        return QString();
    QStringList dirs = xdgDataDirs();
    for (int i = 0; i < dirs.size(); ++i) {
        QFile f(dirs[i] + "/applications/" + id);
        if (f.exists())
            return f.fileName();
    }
    return QString();
}

// Returns the default application for handling the given \a contentType. The
// default application is read from the mimeapps.list. If there is no default
// application, returns an empty string.
QString Internal::defaultAppForContentType(const QString& contentType)
{
    static QHash<QString, QString> defaultApps;

    QStringList dirs = xdgDataDirs();

    // Read the mimeapps.lists / defaults.lists which have changed since we
    // read them the last time.
    QStringList files;
    files << "/applications/mimeapps.list";
    files << "/applications/defaults.list";
    QHash<QString, QString> temp = readChangedKeyValueFiles(dirs, files);

    // Merge the results into our cache
    QHashIterator<QString, QString> it(temp);
    while (it.hasNext()) {
        it.next();
        defaultApps.insert(it.key(), it.value());
    }
    if (defaultApps.contains(contentType))
        return defaultApps.value(contentType);
    QString generalizedType(generalizeMimeType(contentType));
    if (defaultApps.contains(generalizedType))
        return defaultApps.value(generalizedType);
    return QString();
}

// Reads the mimeinfo.cache files and returns the mapping from mime types to
// desktop entries. Tries to decide cleverly whether to re-read the files if
// they might have changed.
const QHash<QString, QStringList>& Internal::mimeApps()
{
    static QHash<QString, QStringList> mimecache;

    QStringList dirs = xdgDataDirs();


    // Read the mimeinfo.caches which have changed since we read them the last
    // time.
    QHash<QString, QString> temp = readChangedKeyValueFiles(dirs,
                                                            QStringList("/applications/mimeinfo.cache"));

    // Merge the changes into our cache
    QHashIterator<QString, QString> it(temp);
    while (it.hasNext()) {
        it.next();
        mimecache.insert(it.key(), it.value().split(";", QString::SkipEmptyParts));
    }
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
        ret << mimeApps()[contentType];

    // Also add more general handlers.
    QString general(generalizeMimeType(contentType));
    if (mimeApps().contains(general))
        ret << mimeApps()[general];

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
    return defaultActionForFile(fileUri, mimeForFile(fileUri));
}

/// Returns the default action for a given \a fileUri, assuming its mime type
/// is \a mimeType. This function can be used even when \a fileUri doesn't
/// exist yet but will be created before trigger() is called, or if you
/// already know the mime type. Note: if the file is a .desktop file, it must
/// exist when this function is called.
Action Action::defaultActionForFile(const QUrl& fileUri, const QString& mimeType_arg)
{
    QString mimeType;
    if (mimeType_arg.isEmpty() || mimeType_arg == "application/octet-stream")
        mimeType = mimeForFile(fileUri);
    else
        mimeType = mimeType_arg;

    if (mimeType.isEmpty())
        return Action();

    QString app = findDesktopFile(defaultAppForContentType(mimeType));
    if (!app.isEmpty()) {
        return createAction(app, QStringList() << fileUri.toEncoded());
    }
    // Fall back to one of the existing actions (if there are some)
    QList<Action> acts = actionsForUri(fileUri.toEncoded(), mimeType);
    if (acts.size() >= 1)
        return acts[0];
    return Action();
}

/// Returns the default action for the given \a files, assuming
/// all their mime types are \a mimeType. This function can be used
/// even when the \a files doesn't exist yet but will be created before
/// trigger() is called, or if you already know the mime type.
Action Action::defaultActionForFile(const QList<QUrl>& uris, const QString& mimeType)
{
    // The function for one URI handles some special cases that don't
    // make sense for multiple files.
    if (uris.size() == 1)
        return defaultActionForFile(uris[0], mimeType);

    QStringList args;
    Q_FOREACH (const QUrl &u, uris) {
        args << u.toEncoded();
    }

    QString app = findDesktopFile(defaultAppForContentType(mimeType));
    if (!app.isEmpty()) {
        return createAction(app, args);
    }
    // Fall back to one of the existing actions (if there are some)
    QList<Action> acts = actionsForUris(args, mimeType);
    if (acts.size() >= 1)
        return acts[0];
    return Action();
}

QList<Action> Internal::actionsForUri(const QString& uri, const QString& mimeType)
{
    return actionsForUris(QStringList() << uri, mimeType);
}

QList<Action> Internal::actionsForUris(const QStringList& uris, const QString& mimeType)
{
    QList<Action> result;

    if (mimeType == DesktopFileMimeType && uris.size() == 1)
        return result << createAction(uris[0], QStringList());

    QStringList appIds = appsForContentType(mimeType);
    Q_FOREACH (const QString& id, appIds) {
        QString app = findDesktopFile(id);
        if (!app.isEmpty())
            result << createAction(app, uris);
    }
    return result;
}

/// Returns the set of applicable actions for a given \a fileUri, based on its
/// content type.
QList<Action> Action::actionsForFile(const QUrl& fileUri)
{
    return actionsForFile(fileUri, mimeForFile(fileUri));
}

/// Returns the set of applicable actions for a given \a fileUri, assuming its
/// content type is \a mimeType.  This function can be used even when \a
/// fileUri doesn't exist but will be created before trigger() is called, or
/// if you already know the mime type.  Note: if the file is a .desktop file,
/// it must exist when this function is called.
QList<Action> Action::actionsForFile(const QUrl& fileUri, const QString& mimeType_arg)
{
    QString mimeType;
    if (mimeType_arg.isEmpty() || mimeType_arg == "application/octet-stream")
        mimeType = mimeForFile(fileUri);
    else
        mimeType = mimeType_arg;

    if (mimeType == DesktopFileMimeType)
        return actionsForUri(fileUri.toLocalFile(), mimeType);

    return actionsForUri(fileUri.toEncoded(), mimeType);
}

/// Returns the set of applicable actions for a given \a mimeType,
/// using the given \a uris as the parameters when the actions are
/// triggered.
QList<Action> Action::actionsForFile(const QList<QUrl>& uris, const QString& mimeType)
{
    if (mimeType == DesktopFileMimeType && uris.size() == 1)
        return actionsForUri(uris[0].toEncoded(), mimeType);

    QStringList args;
    Q_FOREACH (const QUrl &u, uris) {
        args << u.toEncoded();
    }
    return actionsForUris(args, mimeType);
}

/// Returns the pseudo-mimetype of the scheme of \a uri.
QString Internal::mimeForScheme(const QString& uri)
{
    QString mime;
    int n = uri.indexOf(':');
    if (n > 0)
        mime = UriSchemeMimeClass + uri.left(n);
    return mime;
}

/// Returns the x-url-handler of the scheme of \a uri. Method can still change as
/// the mime handler currently doesn't support parameters.
QString mimeForUrl(const QString& uri)
{
    QString mime;

    if (uri.startsWith("http:") || uri.startsWith("https:")) {
        int n = uri.indexOf(':');
        QString domain = uri.mid(n + 3);
        n = domain.indexOf('/');
        domain = domain.mid(n);
        mime = QString("x-url-handler/") + domain;
    }
    return mime;
}

/// Returns the pseudo-mimetypes of the \a param string.  Mime types are found
/// based on exact matching against regexps in the highlighter configuration.
QStringList Internal::mimeForString(const QString& param)
{
    QStringList mimes;
    const QList<QPair<QString, QRegExp> >& cfgList = highlighterConfig();
    for (int i = 0; i < cfgList.size(); ++i) {
        if (cfgList[i].second.exactMatch(param)) {
            mimes << cfgList[i].first;
        }
    }
    return mimes;
}

/// Returns the default action for handling the scheme of the passed \a uri.
/// \sa actionsForScheme().
Action Action::defaultActionForScheme(const QString& uri)
{
    QString mimeType = mimeForScheme(uri);
    QString defApp = findDesktopFile(defaultAppForContentType(mimeType));
    if (!defApp.isEmpty())
        return createAction(defApp, QStringList() << uri);

    // Fall back to one of the existing actions (if there are some)
    QList<Action> acts = actionsForUri(uri, mimeType);
    if (acts.size() >= 1)
        return acts[0];
    return Action();
}

/// Returns the default action for handling the scheme of the passed \a uri.
/// \sa actionsForScheme(). It also converts domain part of uri into
/// "x-url-handler/<domain>" and prioritizes it. Method can still change as
/// the mime handler currently doesn't support parameters.
Action Action::defaultActionForUrl(const QString& uri)
{
    QString urlMimeType = mimeForUrl(uri);
    QString defApp = findDesktopFile(defaultAppForContentType(urlMimeType));
    if (!defApp.isEmpty())
        return createAction(defApp, QStringList() << uri);

    return defaultActionForScheme(uri);
}

/// Returns all actions handling the scheme of the given \a uri.  The uri
/// scheme is mapped to mime types by prefixing it with \c
/// "x-scheme-handler/".  For example an email client may declare to handle
/// the \c "x-scheme-handler/mailto" mimetype and a browser then just
/// triggers the returned Action to activate a \c mailto: link.
QList<Action> Action::actionsForScheme(const QString& uri)
{
    QList<Action> result;
    Q_FOREACH (const QString& app, appsForContentType(mimeForScheme(uri))) {
        result << createAction(findDesktopFile(app), QStringList() << uri);
    }
    return result;
}

/// Returns all actions handling the scheme of the given \a uri.  The uri
/// scheme is mapped to mime types by prefixing it with \c
/// "x-scheme-handler/".  For example an email client may declare to handle
/// the \c "x-scheme-handler/mailto" mimetype and a browser then just
/// triggers the returned Action to activate a \c mailto: link.
/// Additionally domain part of uri is converted into
/// "x-url-handler/<domain>".
QList<Action> Action::actionsForUrl(const QString& uri)
{
    QList<Action> result;

    QString urlMimeType = mimeForUrl(uri);
    Q_FOREACH (const QString& app, appsForContentType(urlMimeType)) {
        result << createAction(findDesktopFile(app), QStringList() << uri);
    }

    result.append(actionsForScheme(uri));
    return result;
}

/// Returns the default action for handling the passed \a param.
/// \sa actionsForString().
Action Action::defaultActionForString(const QString& param)
{
    QStringList mimeTypes = mimeForString(param);
    Q_FOREACH (const QString& mimeType, mimeTypes) {
        QString def = findDesktopFile(defaultAppForContentType(mimeType));
        if (!def.isEmpty())
            return createAction(def,
                                QStringList() << param);
    }
    // Fall back to one of the existing actions (if there are some)
    QList<Action> acts = actionsForString(param);
    if (acts.size() >= 1)
        return acts[0];
    return Action();
}

/// Returns all actions handling the given string \a param.  Dispatching is done
/// based on exact matching against the regexpx of highlighter configuration.
QList<Action> Action::actionsForString(const QString& param)
{
    QStringList mimeTypes = mimeForString(param);
    QList<Action> result;
    Q_FOREACH (const QString& mimeType, mimeTypes) {
        QStringList apps = appsForContentType(mimeType);
        Q_FOREACH (const QString& appid, apps) {
            QString app = findDesktopFile(appid);
            if (!app.isEmpty()) {
                result << createAction(app,
                                       QStringList() << param);
            }
        }
    }
    return result;
}

QList<Action> actionsForMime(const QString& mimeType)
{
    QList<Action> result;
    QStringList appIds = appsForContentType(mimeType);
    Q_FOREACH (const QString& id, appIds) {
        result << createAction(findDesktopFile(id),
                               QStringList());
    }
    return result;
}

Action defaultActionForMime(const QString& mimeType)
{
    QString app = findDesktopFile(defaultAppForContentType(mimeType));
    if (!app.isEmpty())
        return createAction(app,
                            QStringList());
    return Action();
}

} // end namespace
