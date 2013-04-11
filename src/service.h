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

#ifndef SERVICE_H
#define SERVICE_H

#include <QObject>
#include <QString>
#include <QHash>

class QDBusInterface;

namespace ContentAction
{

class ServiceResolver : public QObject
{
    Q_OBJECT
public:
    ServiceResolver();
    ~ServiceResolver();
    QDBusInterface* implementor(const QString& interface);
    QDBusInterface* implementorForAction(const QString& action, QString& method);

private Q_SLOTS:
    void onServiceAvailable(QString, QString);
    void onServiceUnavailable(QString);

private:
    QString implementorName(const QString&);

    QHash<QString, QString> resolved;
    QHash<QString, QDBusInterface*> proxies;
};

ServiceResolver& resolver();

}
#endif
