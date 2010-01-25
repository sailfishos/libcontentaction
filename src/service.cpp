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
#include <QDebug>

namespace ContentAction
{

ServiceResolver::ServiceResolver()
    : mapperProxy(new QDBusInterface("com.nokia.DuiServiceFw", "/",
                                  "com.nokia.DuiServiceFwIf", QDBusConnection::sessionBus()))
{
    qDebug() << "conn:" << connect(mapperProxy, SIGNAL(serviceAvailable(QString,QString)),
                                   this, SLOT(onServiceAvailable(QString,QString)));

    qDebug() << "conn:" << connect(mapperProxy, SIGNAL(serviceUnavailable(QString)),
            this, SLOT(onServiceUnavailable(QString)));
}

void ServiceResolver::onServiceAvailable(QString implementor, QString interface)
{
    qDebug() << "service available" << interface << implementor;
    // We don't know whether the service that became available is now the
    // preferred implementor of some service. So now we don't know anything
    // any more.
    resolved.clear();
    proxies.clear();
}

void ServiceResolver::onServiceUnavailable(QString implementor)
{
    qDebug() << "service available" << implementor;
    // FIXME: was the interpretation correct; the paremeter is the implementor?
    QList<QString> interfaces = resolved.keys(implementor);
    foreach (const QString interface, interfaces)
        resolved.remove(interface);
    if (proxies.contains(implementor))
        delete proxies.take(implementor);
}

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

QDBusInterface* ServiceResolver::implementor(const QString& interface)
{
    /* NOTE: this is an over-simplistic implementation to be refined later. */
    QString name = implementorName(interface);
    if (!proxies.contains(name)) {
        proxies.insert(name, new QDBusInterface(name, "/", interface, QDBusConnection::sessionBus()));
    }
    return proxies[name];
}

}
