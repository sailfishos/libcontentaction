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
#include <QUrl>

namespace ContentAction
{

struct Match;

class Action
{
public:
    void setAsDefault();
    bool isDefault() const;
    bool canBeDefault() const;

    bool isValid() const;
    QString name() const;

    static Action defaultAction(const QString& uri);
    static Action defaultAction(const QStringList& uris);
    static Action defaultActionForFile(const QUrl& fileUri);

    static QList<Action> actions(const QString& uri);
    static QList<Action> actions(const QStringList& uris);
    static QList<Action> actionsForFile(const QUrl& fileUri);

    static QList<Match> highlight(const QString& text);

    struct DefaultPrivate;

    Action(DefaultPrivate *priv);
    Action();
    Action(const Action& other);
    ~Action();
    Action& operator=(const Action& other);

public slots:
    void trigger() const;

private:
    DefaultPrivate* d; /// Pimpl pointer

    // TODO: get rid of this
    friend class TrackerPrivate;
};

struct Match {
    QList<Action> actions; ///< list of applicable actions
    int start, end; ///< [start, end) determines the matching substring

    bool operator<(const Match& other) const;
};

} // end namespace
#endif
