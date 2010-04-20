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
#include <QSharedPointer>

class MLabel;

namespace ContentAction
{

struct Match;
struct ActionPrivate;

class Action
{
public:
    bool isValid() const;
    QString name() const;
    QString localizedName() const;
    QString icon() const;

    static Action defaultAction(const QString& uri);
    static Action defaultAction(const QStringList& uris);
    static Action defaultActionForFile(const QUrl& fileUri);
    static Action defaultActionForFile(const QUrl& fileUri, const QString& mimeType);
    static Action defaultActionForScheme(const QString& uri);

    static QList<Action> actions(const QString& uri);
    static QList<Action> actions(const QStringList& uris);
    static QList<Action> actionsForFile(const QUrl& fileUri);
    static QList<Action> actionsForFile(const QUrl& fileUri, const QString& mimeType);
    static QList<Action> actionsForScheme(const QString& uri);

    static QList<Match> highlight(const QString& text);

    Action();
    ~Action();
    Action(const Action& other);
    Action& operator=(const Action& other);

    void trigger() const;

private:
    Action(ActionPrivate* priv);

    QSharedPointer<ActionPrivate> d;

    friend Action createAction(const QString& desktopFileId, const QStringList& params);
};

struct Match {
    QList<Action> actions; ///< list of applicable actions
    int start, end; ///< [start, end) determines the matching substring

    bool operator<(const Match& other) const;
};

QList<Action> actionsForMime(const QString& mimeType);
void setMimeDefault(const QString& mimeType, const Action& action);
void setMimeDefault(const QString& mimeType, const QString& app);
void resetMimeDefault(const QString& mimeType);

void highlightLabel(MLabel *label);
void highlightLabel(MLabel *label, QStringList typesToHighlight);
void dehighlightLabel(MLabel *label);

} // end namespace
#endif
