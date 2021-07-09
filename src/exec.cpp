/*
 * Copyright (C) 2010 Nokia Corporation.
 * Copyright (C) 2013 - 2020 Jolla Ltd.
 * Copyright (C) 2021 Open Mobile Platform LLC.
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
        if (boosterType == NULL || g_strcmp0(boosterType, "no-invoker") == 0) {
            // Default booster type is "generic". This can be overridden via
            // "X-Nemo-Application-Type=<boostertype>".
            // "no-invoker" is synonymous to "generic"
            g_free(boosterType);
            boosterType = g_strdup("generic");
        }

        gchar *application = g_strdup(QFileInfo(desktopEntry->fileName())
                .completeBaseName().toLocal8Bit().constData());

        gchar *singleInstanceValue = g_key_file_get_string(keyFile, "Desktop Entry",
                "X-Nemo-Single-Instance", NULL);
        bool singleInstance = true;
        if (g_strcmp0(singleInstanceValue, "no") == 0) {
            // Default is to use single-instance launching. This can be disabled
            // by using "X-Nemo-Single-Instance=no".
            singleInstance = false;
        }

        gchar *invokerString = g_strdup_printf("invoker --type=%s --id=%s %s %s",
                boosterType,
                application,
                singleInstance ? "--single-instance" : "",
                execString);
        g_key_file_set_string(keyFile, "Desktop Entry", "Exec", invokerString);
        g_free(invokerString);

        g_free(boosterType);
        g_free(application);
        g_free(singleInstanceValue);
    }

    g_free(execString);

    if (!execError)
        appInfo = g_desktop_app_info_new_from_keyfile(keyFile);

    if (appInfo == 0) {
        LCA_WARNING << "invalid desktop file" << desktopEntry->fileName();
    }
    g_clear_error(&execError);
    g_key_file_free(keyFile);
}

ExecPrivate::~ExecPrivate()
{
    if (appInfo) {
        g_object_unref(appInfo);
    }
}

static void setupProcessIds(gpointer)
{
    const gid_t gid = getgid();
    const uid_t uid = getuid();

    if (setregid(gid, gid) < 0) {
        fprintf(stderr, "Could not setegid to actual group\n");
    } else if (setreuid(uid, uid) < 0) {
        fprintf(stderr, "Could not seteuid to actual user\n");
    } else {
        return;
    }
    ::_exit(EXIT_FAILURE);
}

void ExecPrivate::trigger(bool) const
{
    if (!appInfo) {
        LCA_WARNING << "Exec action triggered without proper appInfo";
        return;
    }
    // Ignore whether the user wanted to wait for the application to start.
    GError *error = 0;
    GList *uris = NULL;

    Q_FOREACH (const QString& param, params)
        uris = g_list_append(uris, g_strdup(param.toUtf8().constData()));

    g_desktop_app_info_launch_uris_as_manager(
                appInfo,
                uris,
                NULL,
                G_SPAWN_SEARCH_PATH,
                setupProcessIds,
                NULL,
                NULL,
                NULL,
                &error);
    if (error != NULL) {
        LCA_WARNING << "cannot trigger: " << error->message;
        g_error_free(error);
    }
    g_list_free_full(uris, g_free);
}

} // end namespace ContentAction
