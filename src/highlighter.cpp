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
#include "contentaction.h"
#include "internal.h"

#include <QDebug>
#include <QRegularExpression>
#include <QPair>
#include <QDBusInterface>
#include <QCoreApplication>

namespace ContentAction {

using namespace ContentAction::Internal;

/// Highlights fragments of \a text which have applicable actions.
/// Returns a list of Match objects.  \deprecated Use
/// ContentAction::Action::findHighlights() instead.
QList<Match> Action::highlight(const QString& text)
{
    const QList<QPair<QString, QRegularExpression> >& cfg = highlighterConfig();
    QList<Match> result;

    for (int i = 0; i < cfg.size(); ++i) {
        QStringList apps = appsForContentType(cfg[i].first);
        const QRegularExpression &re = cfg[i].second;
        QRegularExpressionMatchIterator it = re.globalMatch(text);

        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            int pos = match.capturedStart();
            int length = match.capturedLength();
            Match m;
            m.start = pos;
            m.end = pos + length;

            Q_FOREACH (const QString& app, apps) {
                const QString &desktop = findDesktopFile(app);
                if (desktop != "")
                    m.actions << createAction(desktop, QStringList() << match.captured());
            }
            result << m;
        }
    }
    return result;
}

bool Match::operator<(const Match& other) const
{
    return (this->start < other.start)
            || ((this->start == other.start) && (this->end < other.end));
}

/// Finds fragments of \a text which have applicable actions.  Returns a list of
/// (start, length) pairs which identify the locations of the fragments.  The
/// fragments can be passed to ContentAction::Action::actionsForString() and
/// ContentAction::Action::defaultActionForString() for finding out the
/// applicable actions and the default action.
QList<QPair<int, int> > Action::findHighlights(const QString& text)
{
    QRegularExpression regexp = masterRegexp();

    QList<QPair<int, int> > result;

    if (regexp.pattern() == "(?:)") {
        // The regexp doesn't have any real content -> no matches. "(?:)" is
        // what masterRegexp() will return if there are no regexps to combine
        // together.
        return result;
    }

    QPair<int, int> next;
    int pos = 0;
    while (true) {
        next = findNextHighlight(text, pos);
        if (next.first == -1)
            break;

        result << QPair<int, int>{next.first, next.second};

        pos = next.first + next.second;
        if (next.second == 0)
            // regexp matched an empty string, avoid the inifinite loop
            ++pos;
    }

    return result;
}

/// Finds the next fragment of \a text, starting from \a start, which has
/// applicable actions.  Returns a (start, length) pair which identifies the
/// location of the fragment.  Returns (-1, -1) if no such fragment can be
/// found.  The fragment can be passed to
/// ContentAction::Action::actionsForString() and
/// ContentAction::Action::defaultActionForString() for finding out the
/// applicable actions and the default action.
QPair<int, int> Action::findNextHighlight(const QString& text, int start)
{
    QRegularExpression regexp = masterRegexp();

    if (regexp.pattern() == "(?:)") {
        // The regexp doesn't have any real content -> no matches. "(?:)" is
        // what masterRegexp() will return if there are no regexps to combine
        // together.
        return qMakePair<int, int>(-1, -1);
    }
    QRegularExpressionMatch match = regexp.match(text, start);
    if (!match.hasMatch()) {
        return qMakePair<int, int>(-1, -1);
    }

    return qMakePair(match.capturedStart(), match.capturedLength());
}

} // end namespace
