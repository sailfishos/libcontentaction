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

#include <MDesktopEntry>

#include <QDBusInterface>
#include <QDBusMessage>
#include <QStringList>
#include <QDBusPendingCallWatcher>

#define MAPPER_SERVICENAME "com.nokia.MServiceFw"
#define MAPPER_PATH "/"
#define MAPPER_INTERFACE "com.nokia.MServiceFwIf"

using namespace ContentAction::Internal;

namespace ContentAction
{

ServiceFwPrivate::ServiceFwPrivate(QSharedPointer<MDesktopEntry> desktopEntry,
                                   const QStringList& params)
    : DefaultPrivate(desktopEntry, params),
      serviceFwMethod(desktopEntry->value(XMaemoMethodKey))
{
}

void ServiceFwPrivate::trigger(bool wait) const
{
    QString method;
    QDBusInterface *proxy = resolver().implementorForAction(serviceFwMethod, method);
    if (!proxy)
        return;
    QDBusPendingCallWatcher watcher(proxy->asyncCall(method, params));

    if (wait) {
        watcher.waitForFinished();
        if (watcher.isError()) {
            LCA_WARNING << "error reply from service implementor"
                        << watcher.error().message()
                        << "when trying to call" << serviceFwMethod
                        << "on" << proxy->service();
        }
    }
}

ServiceResolver& resolver()
{
    static ServiceResolver resolver;
    return resolver;
}

ServiceResolver::ServiceResolver()
{
    QDBusConnection conn = QDBusConnection::sessionBus();

    conn.connect(MAPPER_SERVICENAME,
                 MAPPER_PATH,
                 MAPPER_INTERFACE,
                 "serviceAvailable",
                 this,
                 SLOT(onServiceAvailable(QString,QString)));
    conn.connect(MAPPER_SERVICENAME,
                 MAPPER_PATH,
                 MAPPER_INTERFACE,
                 "serviceUnavailable",
                 this,
                 SLOT(onServiceUnavailable(QString,QString)));
}

ServiceResolver::~ServiceResolver()
{
    Q_FOREACH (QDBusInterface* proxy, proxies)
        delete proxy;
    proxies.clear();
}

/// A slot connected to the serviceAvailable signal from Meego service mapper.
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

/// A slot connected to the serviceUnavailable signal from Meego service mapper.
void ServiceResolver::onServiceUnavailable(QString implementor)
{
    // Check which interfaces now become unusable
    QStringList interfaces = resolved.keys(implementor);
    Q_FOREACH (const QString& interface, interfaces)
        resolved.remove(interface);
    if (proxies.contains(implementor))
        delete proxies.take(implementor);
}

/// Returns the name of the current implementor of an interface. If an error
/// occurs (e.g., we cannot connect to the Meego service mapper), returns an
/// empty string.
QString ServiceResolver::implementorName(const QString& interface)
{
    if (resolved.contains(interface))
        return resolved[interface];

    // A blocking call
    QDBusMessage message =
        QDBusMessage::createMethodCall(MAPPER_SERVICENAME, MAPPER_PATH,
                                       MAPPER_INTERFACE, "serviceName");
    message.setArguments(QVariantList() << interface);

    QDBusMessage reply = QDBusConnection::sessionBus().call(message);
    if (reply.type() != QDBusMessage::ReplyMessage || reply.arguments().size() == 0) {
        LCA_WARNING << "invalid reply from service mapper" << reply.errorName();
        return "";
    }
    QString service = reply.arguments()[0].toString();
    if (service.size() == 0) {
        // don't insert to the "resolved" map
        return "";
    }
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
