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
    : DefaultPrivate(desktopEntry, params), appInfo(0)
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

    gchar *execString = g_key_file_get_string(keyFile,
            "Desktop Entry",
            "Exec",
            &execError);

    // Since the list of terminals is hard coded in glib, check here to see if
    // Terminal=true has been set
    if (desktopEntry->terminal()) {
        // Set it to false
        g_key_file_set_boolean(keyFile, "Desktop Entry", "Terminal", false);
        // We will just prepend fingerterm -e before the actual command here
        if (!execError) {
            gchar* terminalString = g_strdup_printf("fingerterm -e %s",execString);
            g_key_file_set_string(keyFile, "Desktop Entry", "Exec", terminalString);
            g_free(terminalString);
        }
    }

    if (!execError && g_strstr_len(execString, -1, "invoker") != execString &&
            g_strstr_len(execString, -1, "/usr/bin/invoker") != execString) {
        // Force invoker usage if invoker isn't specified in Exec= line already

        gchar *boosterType = g_key_file_get_string(keyFile, "Desktop Entry",
                "X-Nemo-Application-Type", NULL);
        if (boosterType == NULL) {
            // Default booster type is "generic". This can be overridden via
            // "X-Nemo-Application-Type=<boostertype>".
            boosterType = g_strdup("generic");
        }

        gchar *singleInstanceValue = g_key_file_get_string(keyFile, "Desktop Entry",
                "X-Nemo-Single-Instance", NULL);
        bool singleInstance = true;
        if (g_strcmp0(singleInstanceValue, "no") == 0) {
            // Default is to use single-instance launching. This can be disabled
            // by using "X-Nemo-Single-Instance=no".
            singleInstance = false;
        }

        // Set X-Nemo-Application-Type to "no-invoker" to disable invoker
        if (g_strcmp0(boosterType, "no-invoker") != 0) {
            gchar *invokerString = g_strdup_printf("invoker --type=%s %s %s",
                    boosterType,
                    singleInstance ? "--single-instance" : "",
                    execString);
            g_key_file_set_string(keyFile, "Desktop Entry", "Exec", invokerString);
            g_free(invokerString);
        }

        g_free(boosterType);
        g_free(singleInstanceValue);
    }

    g_free(execString);

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
