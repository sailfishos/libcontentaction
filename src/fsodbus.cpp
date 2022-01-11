/*
 * Copyright (c) 2022 Jolla Ltd.
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

#include "internal.h"

#include <MDesktopEntry>
#include <MRemoteAction>
#include <QFileInfo>
#include <QString>

/*
 * Supports Freedesktop.org Desktop Entry D-Bus Activation API with a Sailjail
 * twist.
 *
 * This supports what's basically D-Bus Activation in Desktop Entry
 * specification [1] but this doesn't mandate naming desktop entry files like
 * D-Bus services. Instead you may define X-Sailjail section to set the D-Bus
 * service through OrganizationName and ApplicationName.
 *
 * [1] https://specifications.freedesktop.org/desktop-entry-spec/desktop-entry-spec-latest.html#dbus
 */

using namespace ContentAction::Internal;

namespace {

const QString ApplicationIface = QStringLiteral("org.freedesktop.Application");
const QString ActivateMethod = QStringLiteral("Activate");
const QString OpenMethod = QStringLiteral("Open");

QString desktopEntryPathToBusName(const QString &path)
{
    if (path.endsWith(QStringLiteral(".desktop"))) {
        QString candidate = QFileInfo(path).completeBaseName();
        if (candidate.count('.') >= 2)
            return candidate;
    }
    return QString();
}

QString sailjailSectionToBusName(const QSharedPointer<MDesktopEntry> &desktopEntry)
{
    QString orgName = desktopEntry->value(XSailjailOrganizationNameKey);
    QString appName = desktopEntry->value(XSailjailApplicationNameKey);
    if (!orgName.isEmpty() && orgName.contains('.') && !appName.isEmpty())
        return orgName + '.' + appName;
    return QString();
}

QString busNameToObjectPath(const QString &busName)
{
    QString objectPath = QString('/') + busName;
    objectPath.replace('.', '/');
    objectPath.replace('-', '_');
    return objectPath;
}

} // end anonymous namespace

namespace ContentAction {

FSODBusPrivate::FSODBusPrivate(QSharedPointer<MDesktopEntry> desktopEntry,
                               const QStringList& params)
    : DefaultPrivate(desktopEntry, params)
{
    busName = desktopEntryPathToBusName(desktopEntry->fileName());
    if (busName.isEmpty())
        busName = sailjailSectionToBusName(desktopEntry);
    if (busName.isEmpty()) {
        LCA_WARNING << "invalid desktop file for D-Bus activation" << desktopEntry->fileName();
        valid = false;
    } else {
        objectPath = busNameToObjectPath(busName);
    }
}

void FSODBusPrivate::trigger(bool wait) const
{
    bool hasArguments = !params.isEmpty();
    QVariantList arguments;
    if (hasArguments)
        arguments << params;
    arguments << QVariantMap(); // platform-data, currently unpopulated

    MRemoteAction action(busName, objectPath, ApplicationIface,
                         hasArguments ? OpenMethod : ActivateMethod, arguments);
    if (wait) {
        action.triggerAndWait();
    } else {
        action.trigger();
    }
}

} // end namespace ContentAction
