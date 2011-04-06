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

#ifndef LCA_EXPORT
# if defined(LCA_BUILD)
#  define LCA_EXPORT Q_DECL_EXPORT
# else
#  define LCA_EXPORT Q_DECL_IMPORT
# endif
#endif

class MLabel;
class MDesktopEntry;

namespace ContentAction
{

struct Match;
struct ActionPrivate;

class LCA_EXPORT Action
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
    static Action defaultActionForFile(const QList<QUrl>& fileUris, const QString& mimeType);
    static Action defaultActionForScheme(const QString& uri);
    static Action defaultActionForString(const QString& param);

    static QList<Action> actions(const QString& uri);
    static QList<Action> actions(const QStringList& uris);
    static QList<Action> actionsForFile(const QUrl& fileUri);
    static QList<Action> actionsForFile(const QUrl& fileUri, const QString& mimeType);
    static QList<Action> actionsForFile(const QList<QUrl>& fileUri, const QString& mimeType);
    static QList<Action> actionsForScheme(const QString& uri);
    static QList<Action> actionsForString(const QString& param);

    static Action launcherAction(const QString& app, const QStringList& params);
    static Action launcherAction(QSharedPointer<MDesktopEntry>,
                                 const QStringList& params);

    static QList<Match> highlight(const QString& text);

    Action();
    ~Action();
    Action(const Action& other);
    Action& operator=(const Action& other);

    void trigger() const;
    void triggerAndWait() const;

private:
    Action(ActionPrivate* priv);

    QSharedPointer<ActionPrivate> d;

    friend Action createAction(const QString& desktopFilePath,
                               const QStringList& params);
    friend Action createAction(QSharedPointer<MDesktopEntry> desktopEntry,
                               const QStringList& params);
};

struct LCA_EXPORT Match {
    QList<Action> actions; ///< list of applicable actions
    int start, end; ///< [start, end) determines the matching substring

    bool operator<(const Match& other) const;
};

LCA_EXPORT QList<Action> actionsForMime(const QString& mimeType);
LCA_EXPORT Action defaultActionForMime(const QString& mimeType);
LCA_EXPORT void setMimeDefault(const QString& mimeType, const Action& action);
LCA_EXPORT void setMimeDefault(const QString& mimeType, const QString& app);
LCA_EXPORT void resetMimeDefault(const QString& mimeType);

LCA_EXPORT void highlightLabel(MLabel *label);
LCA_EXPORT void highlightLabel(MLabel *label, QStringList typesToHighlight);
LCA_EXPORT void dehighlightLabel(MLabel *label);

} // end namespace
#endif
