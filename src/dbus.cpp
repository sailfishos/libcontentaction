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

#include <QVariantList>
#include <QDBusInterface>
#include <QDBusPendingCall>

using namespace ContentAction::Internal;

namespace ContentAction {

DBusPrivate::DBusPrivate(DuiDesktopEntry* desktopEntry, const QStringList& params)
    : DefaultPrivate(desktopEntry, params), varArgs(false)
{
    if (desktopEntry->contains(XMaemoServiceKey)) {
        busName = desktopEntry->value(XMaemoServiceKey);

        // Default to com.nokia.DuiApplicationIf.launch but support any
        // interface + method
        QString ifaceMethod = desktopEntry->value(XMaemoMethodKey);
        if (ifaceMethod.isEmpty()) {
            iface = "com.nokia.DuiApplicationIf";
            method = "launch";
            objectPath = "/org/maemo/dui";
        }
        else {
            // Split into method and interface
            int dotIx = ifaceMethod.lastIndexOf(".");
            if (dotIx < 1) {
                LCA_WARNING << "invalid interface.method declaration" << ifaceMethod;
            }
            else {
                // Action, e.g., "com.nokia.video-interface.play"
                iface = ifaceMethod.left(dotIx);
                method = ifaceMethod.right(ifaceMethod.size() - dotIx - 1);
            }

            objectPath = desktopEntry->value(XMaemoObjectPathKey);
            if (objectPath.isEmpty())
                objectPath = "/";
        }
    }
    else if (desktopEntry->contains(XOssoServiceKey)) {
        busName = desktopEntry->value(XOssoServiceKey);
        iface = busName;
        method = "mime_open";
        varArgs = true;
    }
}

void DBusPrivate::trigger() const
{
    if (varArgs) {
        // Call a D-Bus function with a variable length argument list
        QVariantList vargs;
        foreach (const QString& param, params)
            vargs << param;
        QDBusInterface launcher(busName, objectPath, iface);
        launcher.callWithArgumentList(QDBus::NoBlock, method, vargs);
    }
    else {
        // Call a D-Bus function with a string list
        QDBusInterface launcher(busName, objectPath, iface);
        launcher.asyncCall(method, params);
        // FIXME: What if we're launching a non-dui desktop file, and we don't
        // have any func taking a string list; only a func taking nothing?
    }
}

} // end namespace ContentAction
