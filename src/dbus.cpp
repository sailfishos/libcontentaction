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
#include <MRemoteAction>
#include <QProcess>

#include <QVariantList>

using namespace ContentAction::Internal;

namespace ContentAction {

const QString XMaemoFixedArgsKey("Desktop Entry/X-Maemo-Fixed-Args");

DBusPrivate::DBusPrivate(QSharedPointer<MDesktopEntry> desktopEntry,
                         const QStringList& _params)
    : DefaultPrivate(desktopEntry, _params), varArgs(false)
{
    // mime_open  X-Osso-Service
    // meegotouch launch X-Maemo-Service
    // user-defined X-Maemo-Service && X-Maemo-Method (+ X-Maemo-Object-Path)
    //    + fixed args

    if (desktopEntry->contains(XOssoServiceKey)) {
        busName = desktopEntry->value(XOssoServiceKey);
        iface = busName;
        objectPath = "/";
        method = "mime_open";
        varArgs = true;
        return;
    }
    // Now we assume that X-Maemo-Service is present.
    busName = desktopEntry->value(XMaemoServiceKey);
    // Default to com.nokia.MApplicationIf.launch but support any interface
    // + method
    QString ifaceMethod = desktopEntry->value(XMaemoMethodKey);
    if (ifaceMethod.isEmpty()) {
        iface = "com.nokia.MApplicationIf";
        method = "launch";
        objectPath = "/org/maemo/m";
    }
    else {
        // Split into method and interface
        int dotIx = ifaceMethod.lastIndexOf(".");
        if (dotIx < 1) {
            LCA_WARNING << "invalid interface.method declaration" << ifaceMethod;
            return;
        }
        // Action, e.g., "com.nokia.video-interface.play"
        iface = ifaceMethod.left(dotIx);
        method = ifaceMethod.mid(dotIx + 1);
        objectPath = desktopEntry->value(XMaemoObjectPathKey);
        if (objectPath.isEmpty())
            objectPath = "/";
    }
    QStringList fixedArgs = desktopEntry->value(XMaemoFixedArgsKey)
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        .split(';', Qt::SkipEmptyParts);
#else
        .split(';', QString::SkipEmptyParts);
#endif
    fixedArgs.append(params);
    params = fixedArgs;
}

void DBusPrivate::trigger(bool wait) const
{
    QVariantList arguments;
    if (varArgs) {
        // Call a D-Bus function with a variable length argument list
        for (const QString &param : params) {
            arguments << param;
        }
    } else {
        arguments.append(params);
    }

    MRemoteAction action(busName, objectPath, iface, method, arguments);
    if (wait) {
        action.triggerAndWait();
    } else {
        action.trigger();
    }
}

} // end namespace ContentAction
