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

// unused gconf code, might go to trash

#include "internal.h"

#include <gconf/gconf.h>
#include <gconf/gconf-client.h>

namespace ContentAction {
namespace Internal {

// initialized on the first request
static GConfClient *Gconf = 0;

static const QString GCONF_KEY_PREFIX("/Dui/contentaction/");

/// Returns the per-class default action. If there is no default
/// action for that class, returns an empty string.
QString Internal::defaultActionFromGConf(const QString& klass)
{
    if (Gconf == 0) {
        g_type_init(); // XXX: needed?
        Gconf = gconf_client_get_default();
    }

    // Query the value from GConf
    char* escaped = gconf_escape_key(klass.toUtf8().constData(), -1);
    QString key = GCONF_KEY_PREFIX + QString::fromAscii(escaped);

    GError* error = NULL;
    GConfValue* value = gconf_client_get(Gconf, key.toAscii().constData(), &error);

    g_free(escaped);

    if (error) {
        LCA_WARNING << "Error getting data from GConf:" << error->message;
        g_error_free(error);
        return "";
    }

    if (value == 0) {
        // The key doesn't exist; no default action for the class
        return "";
    }

    const char* valueString = gconf_value_get_string(value); // shouldn't be freed

    QString action = "";
    if (valueString) {
        action = QString::fromUtf8(valueString);
    }

    gconf_value_free(value);
    return action;
}

bool Internal::setDefaultAction(const QString& klass, const QString& action)
{
    if (Gconf == 0) {
        g_type_init(); // XXX: needed?
        Gconf = gconf_client_get_default();
    }

    // Set the class - action pair to GConf
    char* escaped = gconf_escape_key(klass.toUtf8().constData(), -1);
    QString key = GCONF_KEY_PREFIX + QString::fromAscii(escaped);

    GError* error = NULL;

    gconf_client_set_string(Gconf, key.toAscii().constData(),
                            action.toUtf8().constData(), &error);
    g_free(escaped);

    if (error) {
        LCA_WARNING << "Error setting data to GConf:" << error->message;
        g_error_free(error);
        return false;
    }
    return true;
}

} // end namespace Internal
} // end namespace ContentAction
