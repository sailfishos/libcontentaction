/*
 * Copyright (C) 2009 Nokia Corporation.
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

#ifndef CONTENTACTION_H
#define CONTENTACTION_H

#include <QList>
#include <QString>
#include <QStringList>

struct ContentActionPrivate
{
    bool valid;
    QString uri;
    QStringList classes;
    QString action; // <service fw interface>.<method>
};

class ContentAction
{
public:
    ContentAction();
    ContentAction(const ContentAction& other);
    ~ContentAction();
    ContentAction& operator=(const ContentAction& other);

    void trigger() const;
    void setAsDefault();
    bool isDefault() const;
    bool canBeDefault() const;

    static ContentAction defaultAction(const QString& uri);
    static ContentAction defaultAction(const QStringList& uris);

    static QList<ContentAction> actions(const QString& uri);
    static QList<ContentAction> actions(const QStringList& uris);

    static QStringList classesOf(const QString& uri);
    static QStringList actionsForClass(const QString& klass);

private:
    ContentAction(const QString& uri, const QStringList& classes,
                  const QString& action);

    ContentActionPrivate* d;
};

#endif
