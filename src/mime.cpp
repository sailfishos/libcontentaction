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

} // end namespace
