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
 */
#include "contentaction.h"
#include "internal.h"

#include <QRegExp>
#include <QDebug>
#include <QModelIndex>
#include <QAbstractListModel>

typedef QPair<QString, QRegExp> MimeAndRegexp;
typedef QList<MimeAndRegexp> MimesAndRegexps;

namespace {

using namespace ContentAction;
using namespace ContentAction::Internal;

MimesAndRegexps regExpsInUse()
{
    // Returns the regexps for which we have actions.
    static MimesAndRegexps mars;
    static bool read = false;
    if (!read) {
        QListIterator<QPair<QString, QRegExp> > iter(highlighterConfig());
        while (iter.hasNext()) {
            const QPair<QString, QRegExp> &mar = iter.next();
            if (!appsForContentType(mar.first).isEmpty())
                mars += MimeAndRegexp(mar.first, mar.second);
        }
        read = true;
    }
    return mars;
}

QRegExp combine(const MimesAndRegexps &mars)
{
    QString re("(?:");
    bool first = true;
    Q_FOREACH (const MimeAndRegexp &mr, mars) {
        if (!first)
            re += '|';
        re += mr.second.pattern();
        first = false;
    }
    re += ")";
    return QRegExp(re);
}

} // end anon namespace

namespace ContentAction {
namespace Internal {

QRegExp masterRegexp()
{
    static QRegExp master;
    static bool read = false;
    if (!read) {
        master = combine(regExpsInUse());
        read = true;
    }
    return master;
}

} // end namespace Internal
} // end namespace ContentAction

Q_DECLARE_METATYPE(ContentAction::Action);

