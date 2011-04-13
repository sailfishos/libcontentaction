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
#include <QRegExp>
#include <QDBusInterface>
#include <QCoreApplication>

namespace ContentAction {

using namespace ContentAction::Internal;

/// Highlights fragments of \a text which have applicable actions.  Returns a
/// list of Match objects.  \deprecated Use
/// ContentAction::Action::findHighlights() instead.
QList<Match> Action::highlight(const QString& text)
{
    const QList<QPair<QString, QRegExp> >& cfg = highlighterConfig();
    QList<Match> result;

    for (int i = 0; i < cfg.size(); ++i) {
	const QRegExp &re = cfg[i].second;
        QStringList apps = appsForContentType(cfg[i].first);
        int pos = 0;
        while ((pos = re.indexIn(text, pos)) != -1) {
            int l = re.matchedLength();
            Match m;
            m.start = pos;
            m.end = pos + l;

            Q_FOREACH (const QString& app, apps) {
                const QString &desktop = findDesktopFile(app);
                if (desktop != "")
                    m.actions << createAction(desktop, QStringList() << re.cap());
            }
            result << m;
            pos += l;
            if (l == 0)
                ++pos;
        }
    }
    return result;
}

bool Match::operator<(const Match& other) const
{
    return (this->start < other.start) ||
        ((this->start == other.start) && (this->end < other.end));
}

/// Finds fragments of \a text which begin on the range [\a start, \a end[ and
/// which have applicable actions.  Returns a list of (start, length) pairs
/// which identify the locations of the fragments.  If \a end is negative, the
/// whole string \a text is used.  The fragments can be passed to
/// ContentAction::Action::actionsForString() and
/// ContentAction::Action::defaultActionForString() for finding out the
/// applicable actions and the default action.
QList<QPair<int, int> > Action::findHighlights(const QString& text,
                                               int start, int end)
{
    QRegExp regexp = masterRegexp();

    QList<QPair<int, int> > result;

    if (regexp.pattern() == "(?:)") {
        // The regexp doesn't have any real content -> no matches. "(?:)" is
        // what masterRegexp() will return if there are no regexps to combine
        // together.
        return result;
    }

    int pos = start;

    // Despite the good intention of the range, it's possible that the regexp
    // engine needs to read the whole string (if there are no matches).  This is
    // because 1) there's no way to give the end index to QRegExp::indexIn() 2)
    // we cannot only give a substring of text (we don't know how long the
    // substring would need to be to include all the matches).  (The user's
    // expectation is that if he calls findHighlights(text, 0, n) and
    // findHighlights(text, n, text.length()), all highlights are found, and no
    // highlights are left out because they cross the index "n".)
    while ((pos = regexp.indexIn(text, pos)) != -1) {
        if (end >= 0 && pos >= end)
            break;

        // The match is taken in if its starting point is at range [start, end[.
        int l = regexp.matchedLength();
        result << qMakePair<int, int>(pos,l);
        pos += l;
        if (l == 0)
            ++pos;
    }
    return result;
}

} // end namespace
