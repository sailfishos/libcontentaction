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
 *
 */

#include "internal.h"

#include <DuiDesktopEntry>

#include <QFileInfo>

#include <gio/gdesktopappinfo.h>
#include <gio/gio.h>
#include <glib.h>

namespace ContentAction {

ExecPrivate::ExecPrivate(DuiDesktopEntry* desktopEntry, const QStringList& params)
    : DefaultPrivate(desktopEntry, params)
{
    g_type_init();
    appInfo = G_APP_INFO(g_desktop_app_info_new_from_filename(desktopEntry->fileName().toLocal8Bit().constData()));
}

ExecPrivate::~ExecPrivate()
{
    g_object_unref(appInfo);
}

void ExecPrivate::trigger() const
{
    GError *error = 0;
    GList *uris = NULL;

    foreach (const QString& param, params)
        uris = g_list_append(uris, g_strdup(param.toAscii().constData()));

    g_app_info_launch_uris(appInfo, uris, NULL, &error);
    if (error != NULL) {
        LCA_WARNING << "cannot trigger: " << error->message;
        g_error_free(error);
    }
    g_list_foreach(uris, (GFunc)g_free, NULL);
    g_list_free(uris);
}

} // end namespace ContentAction
