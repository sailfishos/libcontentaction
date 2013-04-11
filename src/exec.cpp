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

#include <MDesktopEntry>

#include <QFileInfo>

#include <gio/gdesktopappinfo.h>
#include <gio/gio.h>
#include <glib.h>

namespace ContentAction {

ExecPrivate::ExecPrivate(QSharedPointer<MDesktopEntry> desktopEntry,
                         const QStringList& params)
    : DefaultPrivate(desktopEntry, params)
{
    g_type_init();
    GError *execError = 0;
    GKeyFile *keyFile;
    GKeyFileFlags flags = G_KEY_FILE_NONE;
    keyFile = g_key_file_new();
    g_key_file_load_from_file(keyFile,
                              desktopEntry->fileName().toLocal8Bit().constData(),
                              flags,
                              NULL);

    // Since the list of terminals is hard coded in glib, check here to see if
    // Terminal=true has been set
    if (desktopEntry->terminal()) {
        // Set it to false
        g_key_file_set_boolean(keyFile, "Desktop Entry", "Terminal", false);
        // We will just prepend meego-terminal -e before the actual command here
        gchar* execString = g_key_file_get_string(keyFile,
                                                  "Desktop Entry",
                                                  "Exec",
                                                  &execError);
        if (!execError) {
            gchar* terminalString = g_strdup_printf("meego-terminal -e %s",execString);
            g_key_file_set_string(keyFile, "Desktop Entry", "Exec", terminalString);
            g_free(terminalString);
        }
        g_free(execString);
    }

    if (!execError)
        appInfo = G_APP_INFO(g_desktop_app_info_new_from_keyfile(keyFile));

    if (appInfo == 0) {
        LCA_WARNING << "invalid desktop file" << desktopEntry->fileName();
    }
    g_clear_error(&execError);
    g_key_file_free(keyFile);
}

ExecPrivate::~ExecPrivate()
{
    g_object_unref(appInfo);
}

void ExecPrivate::trigger(bool) const
{
    // Ignore whether the user wanted to wait for the application to start.
    GError *error = 0;
    GList *uris = NULL;

    Q_FOREACH (const QString& param, params)
        uris = g_list_append(uris, g_strdup(param.toLatin1().constData()));

    g_app_info_launch_uris(appInfo, uris, NULL, &error);
    if (error != NULL) {
        LCA_WARNING << "cannot trigger: " << error->message;
        g_error_free(error);
    }
    g_list_foreach(uris, (GFunc)g_free, NULL);
    g_list_free(uris);
}

} // end namespace ContentAction
