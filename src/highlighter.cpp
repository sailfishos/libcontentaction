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

struct HighlightPrivate: public Action::DefaultPrivate
{
    HighlightPrivate(const QString& match, const QString& action);
    virtual ~HighlightPrivate();
    virtual bool isValid() const;
    virtual QString name() const;
    virtual QString localizedName() const;
    virtual void trigger() const;
    virtual HighlightPrivate* clone() const;

    QString match;
    QString action;
};

HighlightPrivate::HighlightPrivate(const QString& match, const QString& action) :
    match(match), action(action)
{ }

HighlightPrivate::~HighlightPrivate()
{ }

bool HighlightPrivate::isValid() const
{
    return true;
}

QString HighlightPrivate::name() const
{
    return action;
}

QString HighlightPrivate::localizedName() const
{
    return QCoreApplication::translate("ContentAction",
                                       name().toAscii().constData(),
                                       "", QCoreApplication::CodecForTr);
}

void HighlightPrivate::trigger() const
{
    QString method;
    QDBusInterface *proxy = resolver().implementorForAction(action, method);
    if (!proxy)
        return;
    QDBusMessage reply = proxy->call(method, match);
    if (reply.type() != QDBusMessage::ReplyMessage)
        LCA_WARNING << "error reply from service implementor" << reply.errorName()
                    << "when trying to call" << action
                    << "on" << proxy->service();
}

HighlightPrivate *HighlightPrivate::clone() const
{
    return new HighlightPrivate(match, action);
}

/// Highlights fragments of \a text which have applicable actions.  Returns a
/// list of Match objects.
QList<Match> Action::highlight(const QString& text)
{
    const HighlighterMap& cfg = highlighterConfig();
    QList<Match> result;

    foreach (const QString& sre, cfg.keys()) {
        QRegExp re(sre);
        int pos = 0;
        while ((pos = re.indexIn(text, pos)) != -1) {
            int l = re.matchedLength();
            Match m;
            m.start = pos;
            m.end = pos + l;
            foreach (const QString& act, cfg[sre]) {
                m.actions << highlightAction(re.cap(), act);
            }
            result << m;
            pos += l;
        }
    }
    return result;
}

bool Match::operator<(const Match& other) const
{
    return (this->start < other.start) ||
        ((this->start == other.start) && (this->end < other.end));
}


Action Internal::highlightAction(const QString& match,
                                 const QString& action)
{
    return Action(new HighlightPrivate(match, action));
}

} // end namespace
