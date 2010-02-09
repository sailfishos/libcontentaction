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

namespace ContentAction {

using namespace ContentAction::Internal;

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

Action Internal::mimeAction(const QUrl& fileUri, GAppInfo* appInfo)
{
    return Action(new MimePrivate(fileUri, appInfo));
}

/// Returns the default action for a given \a file, based on its content type.
Action Action::defaultActionForFile(const QUrl& fileUri)
{
    QByteArray uri = fileUri.toEncoded();
    char* contentType = contentTypeForFile(uri.constData());
    if (contentType == NULL)
        return Action();
    GAppInfo* appInfo = g_app_info_get_default_for_type(contentType, TRUE);
    g_free(contentType);
    if (appInfo == NULL)
        return Action();
    return mimeAction(fileUri, appInfo);
}


/// Returns the set of applicable actions for a given \a file, based on its
/// content type.
QList<Action> Action::actionsForFile(const QUrl& fileUri)
{
    QList<Action> result;
    QByteArray uri = fileUri.toEncoded();
    char* contentType = contentTypeForFile(uri.constData());
    if (contentType == NULL)
        return result;

    GList* appInfoList = g_app_info_get_all_for_type(contentType);
    g_free(contentType);
    GList* cur = appInfoList;
    while (cur != NULL) {
        result << mimeAction(fileUri, g_app_info_dup((GAppInfo*)cur->data));
        g_object_unref(cur->data);
        cur = g_list_next(cur);
    }
    g_list_free(appInfoList);
    return result;
}

/// Returns the content type of the given file, or an empty string if it cannot
/// be retrieved.
char* Internal::contentTypeForFile(const char* fileUri)
{
    g_type_init();
    GError *error = 0;
    GFile *file = g_file_new_for_uri(fileUri);

    GFileInfo *fileInfo = g_file_query_info(file,
                                            "standard::*",
                                            G_FILE_QUERY_INFO_NONE,
                                            NULL,
                                            &error);
    if (error != 0) {
        g_error_free(error);
        g_object_unref(file);
        return NULL;
    }
    char *contentType = g_strdup(g_file_info_get_content_type(fileInfo));
    g_object_unref(fileInfo);
    g_object_unref(file);
    return contentType;
}

} // end namespace
