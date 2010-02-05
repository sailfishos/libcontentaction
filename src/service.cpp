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

#include "service.h"
#include "internal.h"

#include <QDBusInterface>
#include <QDBusMessage>
#include <QStringList>
#include <QDebug>

namespace ContentAction
{

ServiceResolver::ServiceResolver()
    : mapperProxy(new QDBusInterface("com.nokia.DuiServiceFw", "/",
                                  "com.nokia.DuiServiceFwIf", QDBusConnection::sessionBus()))
{
    connect(mapperProxy, SIGNAL(serviceAvailable(QString,QString)),
                                   this, SLOT(onServiceAvailable(QString,QString)));

    connect(mapperProxy, SIGNAL(serviceUnavailable(QString)),
            this, SLOT(onServiceUnavailable(QString)));
}

ServiceResolver::~ServiceResolver()
{
    foreach (QDBusInterface* proxy, proxies)
        delete proxy;
    proxies.clear();
    delete mapperProxy;
}

/// A slot connected to the serviceAvailable signal from Dui service mapper.
void ServiceResolver::onServiceAvailable(QString implementor, QString interface)
{
    // Now a service become available for an interface, but we cannot know
    // wheter it is now the *preferred* implementor or not. We cannot do
    // anything else but clear our understanging about who's the preferred
    // implementor of the interface.

    // Remove the old implementor (and its proxy)
    if (resolved.contains(interface)) {
        QString oldImplementor = resolved.take(interface);

        if (resolved.keys(oldImplementor).isEmpty())
            // The old implementor implemented only this interface; the proxy
            // can be deleted.
            delete proxies.take(oldImplementor);
    }
}

/// A slot connected to the serviceUnavailable signal from Dui service mapper.
void ServiceResolver::onServiceUnavailable(QString implementor)
{
    // Check which interfaces now become unusable
    QStringList interfaces = resolved.keys(implementor);
    foreach (const QString& interface, interfaces)
        resolved.remove(interface);
    if (proxies.contains(implementor))
        delete proxies.take(implementor);
}

/// Returns the name of the current implementor of an interface. If an error
/// occurs (e.g., we cannot connect to the Dui service mapper), returns an
/// empty string.
QString ServiceResolver::implementorName(const QString& interface)
{
    if (resolved.contains(interface))
        return resolved[interface];

    if (!mapperProxy->isValid()) {
        LCA_WARNING << "cannot connect to service mapper";
        return "";
    }

    // A blocking call
    QDBusMessage reply = mapperProxy->call("serviceName", interface);
    if (reply.type() != QDBusMessage::ReplyMessage || reply.arguments().size() == 0) {
        LCA_WARNING << "invalid reply from service mapper" << reply.errorName();
        return "";
    }
    QString service = reply.arguments()[0].toString();
    resolved.insert(interface, service);
    return service;
}

/// Returns the proxy object for the current implementor of the given
/// interface.
QDBusInterface* ServiceResolver::implementor(const QString& interface)
{
    QString name = implementorName(interface);
    if (!proxies.contains(name)) {
        proxies.insert(name, new QDBusInterface(name, "/", interface, QDBusConnection::sessionBus()));
    }
    return proxies[name];
}

// Splits action to interface.method and looks up the proxy for the interface.
// Returns the method name in \a method.
QDBusInterface *ServiceResolver::implementorForAction(const QString& action,
                                                      QString& method)
{
    // Get the service fw interface from the action name
    int dotIx = action.lastIndexOf(".");
    if (dotIx < 1) {
        LCA_WARNING << "invalid action name" << action;
        return 0;
    }
    // Action, e.g., "com.nokia.video-interface.play"
    QString interface = action.left(dotIx);
    method = action.right(action.size() - dotIx - 1);
    return implementor(interface);
}

}
