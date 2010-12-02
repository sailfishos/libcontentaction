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
#include <QGraphicsGridLayout>
#include <QDebug>
#include <QStringListModel>
#include <QModelIndex>
#include <QAbstractListModel>

#include <MLabel>
#include <MLabelHighlighter>
#include <MPopupList>

typedef QPair<QString, QRegExp> MimeAndRegexp;
typedef QList<MimeAndRegexp> MimesAndRegexps;

namespace {

using namespace ContentAction;
using namespace ContentAction::Internal;

class LCALabelHighlighter: public MCommonLabelHighlighter
{
    Q_OBJECT
public:
    LCALabelHighlighter(const MimesAndRegexps &mars_,
                        QObject *parent = 0);
private Q_SLOTS:
    void doDefaultAction(const QString& match);
    void doPopupActions(const QString& match);
    void doAction(const QModelIndex& ix);
private:
    QStringList matchingMimes(const QString &str) const;
    MimesAndRegexps mars;
};

static QRegExp combine(const MimesAndRegexps &mars)
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
    return QRegExp(re, Qt::CaseInsensitive);
}

LCALabelHighlighter::LCALabelHighlighter(const MimesAndRegexps &mars_,
                                         QObject *parent) :
    MCommonLabelHighlighter(combine(mars_)),
    mars(mars_)
{
    if (parent)
        setParent(parent);
    QObject::connect(this, SIGNAL(clicked(const QString&)),
                     this, SLOT(doDefaultAction(const QString&)));
    QObject::connect(this, SIGNAL(longPressed(const QString&)),
                     this, SLOT(doPopupActions(const QString&)));
}

/// Returns the mimes that match the given string, and are processed by this
/// LCALabelHighlighter.  (Note that the LCALabelHighlighter doesn't necessarily
/// handle all regexps.)
QStringList LCALabelHighlighter::matchingMimes(const QString &str) const
{
    // TODO: the code is a bit duplicate with Internal::mimeForString but not
    // quite.
    QStringList ret;
    Q_FOREACH (const MimeAndRegexp &mr, mars)
        if (mr.second.exactMatch(str))
            ret.append(mr.first);
    return ret;
}

void LCALabelHighlighter::doDefaultAction(const QString& match)
{
    QStringList mimes = matchingMimes(match);
    QString app;
    Q_FOREACH (const QString &mime, mimes) {
        app = findDesktopFile(defaultAppForContentType(mime));
        if (!app.isEmpty()) {
            createAction(app, QStringList() << match).trigger();
            return;
        }
    }
    Q_FOREACH (const QString &mime, mimes) {
        QStringList apps = appsForContentType(mime);
        Q_FOREACH (const QString& appid, apps) {
            app = findDesktopFile(appid);
            if (!app.isEmpty()) {
                createAction(app, QStringList() << match).trigger();
                return;
            }
        }
    }
}

struct ActionListModel: public QAbstractListModel
{
    Q_OBJECT
public:

    enum { ActionRole = Qt::UserRole + 1 };

    ActionListModel(const QList<Action>& actions, QObject *parent = 0) :
        QAbstractListModel(parent),
        actions(actions)
        { }
    virtual int rowCount(const QModelIndex& parent) const
        {
            return actions.count();
        }
    virtual QVariant data(const QModelIndex& ix, int role) const
        {
            QVariant ret;

            if (!ix.isValid() || ix.row() >= actions.count())
                return ret;
            if (role == Qt::DisplayRole)
                ret = actions.at(ix.row()).localizedName();
            else if (role == ActionRole)
                ret.setValue(actions.at(ix.row()));
            return ret;
        }

    QList<Action> actions;
};

void LCALabelHighlighter::doAction(const QModelIndex& ix)
{
    Action a = ix.model()->data(ix, ActionListModel::ActionRole).value<Action>();
    a.trigger();
}

void LCALabelHighlighter::doPopupActions(const QString& match)
{
    qRegisterMetaType<Action>();
    QList<Action> alist;
    QStringList mimes = matchingMimes(match);
    QString app;
    Q_FOREACH (const QString &mime, mimes) {
        Q_FOREACH (const QString &appid, appsForContentType(mime)) {
            app = findDesktopFile(appid);
            if (!app.isEmpty())
                alist << createAction(app, QStringList() << match);
        }
    }

    MPopupList *popuplist = new MPopupList();
    popuplist->setItemModel(new ActionListModel(alist, popuplist));
    popuplist->setTitle(match);
    popuplist->setTitleBarVisible(true);
    connect(popuplist, SIGNAL(clicked(const QModelIndex&)),
            this, SLOT(doAction(const QModelIndex&)));
    connect(popuplist, SIGNAL(finished(int)),
            popuplist, SLOT(deleteLater()));
    popuplist->appear();
}

} // end anon namespace

static void hiLabel(MLabel *label, const MimesAndRegexps &mars)
{
    if (mars.isEmpty())
        return;
    if (label->property("_lca_highlighter").isValid())
        return;
    LCALabelHighlighter *hl = new LCALabelHighlighter(mars, label);
    label->addHighlighter(hl);
    label->setProperty("_lca_highlighter", QVariant::fromValue<void *>(hl));
}

static void unhiliteLabel(MLabel *label)
{
    QVariant prop = label->property("_lca_highlighter");
    if (!prop.isValid())
        return;
    LCALabelHighlighter *hl = static_cast<LCALabelHighlighter *>(prop.value<void *>());
    label->removeHighlighter(hl);
    delete hl;
    label->setProperty("_lca_highlighter", QVariant());
}

/// Attaches possibly several MLabelHighlighter:s to the label, based on the
/// highlighter configuration.  Clicking on a highlighted label invokes the
/// first action defined for the matching pattern.  A long-click causes a
/// popup list to be shown with the possible actions, from where the user may
/// choose one.
void ContentAction::highlightLabel(MLabel *label)
{
    MimesAndRegexps mars;
    QListIterator<QPair<QString, QString> > iter(highlighterConfig());
    while (iter.hasNext()) {
        QPair<QString, QString> mar = iter.next();
        if (!appsForContentType(mar.first).isEmpty())
            mars += qMakePair(mar.first, QRegExp(mar.second, Qt::CaseInsensitive));
    }
    hiLabel(label, mars);
}

/// Similar to highlightLabel() but allows specifying which regexp-types to
/// highlight (e.g. only \c "x-maemo-highlight/mailto"). The order of the \a
/// typesOfHighlight is honoured; the regexps appearing first get the priority
/// when deciding the default action and the order of the actions.
void ContentAction::highlightLabel(MLabel *label,
                                   QStringList typesToHighlight)
{
    MimesAndRegexps mars;
    const QList<QPair<QString, QString> >& cfgList = highlighterConfig();
    QMap<QString, QString> cfgMap;
    for (int i = 0; i < cfgList.size(); ++i)
        cfgMap[cfgList[i].first] = cfgList[i].second;
    Q_FOREACH (const QString& k, typesToHighlight) {
        QString re(cfgMap.value(k, QString()));
        if (re.isEmpty())
            continue;
        if (!appsForContentType(k).isEmpty())
            mars += qMakePair(k, QRegExp(re, Qt::CaseInsensitive));
    }
    hiLabel(label, mars);
}

/// Removes all highlighters attached by highlightLabel() from \a label.
void ContentAction::dehighlightLabel(MLabel *label)
{
    unhiliteLabel(label);
}

Q_DECLARE_METATYPE(ContentAction::Action);

#include "highlight.moc"
