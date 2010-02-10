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

#include <gio/gdesktopappinfo.h>
#include <gio/gio.h>
#include <glib.h>

#include <QSettings>
#include <QDebug>
#include <QFile>
#include <QHash>

namespace ContentAction {

using namespace ContentAction::Internal;

struct MimePrivate: public Action::DefaultPrivate {
    MimePrivate(const QUrl& fileUri, struct _GAppInfo* app);
    virtual ~MimePrivate();
    virtual bool isValid() const;
    virtual QString name() const;
    virtual void trigger() const;
    virtual DefaultPrivate *clone() const;

    QUrl fileUri;
    struct _GAppInfo* appInfo;
};

QString defaultAppForContentType(const QString& contentType);
QStringList appsForContentType(const QString& contentType);

MimePrivate::MimePrivate(const QUrl& fileUri, GAppInfo* appInfo)
    : fileUri(fileUri), appInfo(appInfo)
{ }

MimePrivate::~MimePrivate()
{
    g_object_unref(appInfo);
    appInfo = NULL;
}

bool MimePrivate::isValid() const
{
    return true;
}

QString MimePrivate::name() const
{
    const char* appName = g_app_info_get_name(appInfo);
    return QString(appName);
}

/// Launches the application with gio.
void MimePrivate::trigger() const
{
    GError* error = 0;
    GList* uris = NULL;
    QByteArray uri = fileUri.toEncoded();
    uris = g_list_append(uris, uri.data());

    g_app_info_launch_uris(appInfo, uris, NULL, &error);
    if (error != NULL) {
        LCA_WARNING << "cannot trigger: " << error->message;
        g_error_free(error);
    }
    g_list_free(uris);
}

Action::DefaultPrivate *MimePrivate::clone() const
{
    return new MimePrivate(fileUri, g_app_info_dup(appInfo));
}

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

// Returns an Action, which when triggered launches the application described
// by appInfo, passing fileUri as argument (either directly using GIO or via
// D-Bus).
static Action mimeAction(const QUrl& fileUri, GAppInfo *appInfo)
{
    return Action(new MimePrivate(fileUri, appInfo));
}

/// Returns the default action for a given \a file, based on its content type.
Action Action::defaultActionForFile(const QUrl& fileUri)
{
    QString contentType = contentTypeForFile(fileUri);
    if (contentType.isEmpty())
        return Action();
    GAppInfo *appInfo = g_app_info_get_default_for_type(
        contentType.toAscii().constData(), TRUE);
    if (appInfo == NULL)
        return Action();
    return mimeAction(fileUri, appInfo);
}

/// Returns the set of applicable actions for a given \a file, based on its
/// content type.
QList<Action> Action::actionsForFile(const QUrl& fileUri)
{
    QList<Action> result;

    QString contentType = contentTypeForFile(fileUri);
    if (contentType.isEmpty())
        return result;

    GList* infoList;
    infoList = g_app_info_get_all_for_type(contentType.toAscii().constData());
    for (GList *cur = infoList; cur; cur = cur->next)
        result << mimeAction(fileUri, G_APP_INFO(cur->data));
    g_list_free(infoList);
    return result;
}

QHash<QString, QString> readKeyValues(const QString& fileName)
{
    QHash<QString, QString> ret;
    QFile file(fileName);
    if (!file.exists()) {
        LCA_WARNING << "not found:" << fileName;
        return ret;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        LCA_WARNING << "cannot read:" << fileName;
        return ret;
    }
    QTextStream stream(&file);
    // Ignore the header
    stream.readLine();
    int n = 1;
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        if (line.isNull()) break;
        ++n;
        QStringList keyAndValue = line.split("=", QString::SkipEmptyParts);
        if (keyAndValue.size() < 2) {
            LCA_WARNING << "ignoring a syntax error in" << fileName << "line" << n;
            continue;
        }
        ret.insert(keyAndValue[0], keyAndValue[1]);
    }
    return ret;
}

/// Returns the default application for handling the given \a contentType. The
/// default application is read from the defaults.list. If there is no default
/// application, returns an empty string.
QString defaultAppForContentType(const QString& contentType)
{
    QHash<QString, QString> apps = readKeyValues("/usr/share/applications/defaults.list");
    // FIXME: file name
    if (apps.contains(contentType)) return apps[contentType];
    return "";
}

/// Returns the applications which handle the given \a contentType. The
/// applications are read from the mimeinfo.cache. The file is searched in the
/// default locations. The returned list will contain elements of the form
/// "appname.desktop".
QStringList appsForContentType(const QString& contentType)
{
    QStringList ret;
    // Get all apps handling this content type, FIXME file name
    QHash<QString, QString> appData = readKeyValues("/usr/share/applications/mimeinfo.cache");
    if (appData.contains(contentType))
        ret = appData[contentType].split(";", QString::SkipEmptyParts);

    // Get the default app handling this content type, insert it to the front
    // of the list
    QString defaultApp = defaultAppForContentType(contentType);
    if (!defaultApp.isEmpty()) {
        ret.removeAll(defaultApp);
        ret.prepend(defaultApp);
    }
    return ret;
}


} // end namespace
