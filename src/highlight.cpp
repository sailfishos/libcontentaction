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
private slots:
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
    foreach (const MimeAndRegexp &mr, mars) {
        if (!first)
            re += '|';
        re += mr.second.pattern();
        first = false;
    }
    re += ")";
    return QRegExp(re);
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

QStringList LCALabelHighlighter::matchingMimes(const QString &str) const
{
    QStringList ret;
    foreach (const MimeAndRegexp &mr, mars)
        if (mr.second.exactMatch(str))
            ret.append(mr.first);
    return ret;
}

void LCALabelHighlighter::doDefaultAction(const QString& match)
{
    QStringList mimes = matchingMimes(match);
    QString app;
    foreach (const QString &mime, mimes) {
        app = defaultAppForContentType(mime);
        if (!app.isEmpty()) {
            createAction(findDesktopFile(app), QStringList() << match).trigger();
            return;
        }
    }
    foreach (const QString &mime, mimes) {
        QStringList apps = appsForContentType(mime);
        if (!apps.isEmpty()) {
            createAction(findDesktopFile(apps[0]), QStringList() << match).trigger();
            return;
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
    foreach (const QString &mime, mimes) {
        foreach (const QString &app, appsForContentType(mime))
            alist << createAction(findDesktopFile(app), QStringList() << match);
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
    QHashIterator<QString, QString> iter(highlighterConfig());
    while (iter.hasNext()) {
        iter.next();
        // iter.key == mime type, iter.value == regexp
        if (!appsForContentType(iter.key()).isEmpty())
            mars += qMakePair(iter.key(), QRegExp(iter.value()));
    }
    hiLabel(label, mars);
}

/// Similar to highlightLabel() but allows specifying which regexp-types to
/// highlight (e.g. only \c "x-maemo-highlight/mailto").
void ContentAction::highlightLabel(MLabel *label,
                                   QStringList typesToHighlight)
{
    MimesAndRegexps mars;
    const QHash<QString, QString>& cfg = highlighterConfig();
    foreach (const QString& k, typesToHighlight) {
        QString re(cfg.value(k, QString()));
        if (re.isEmpty())
            continue;
        if (!appsForContentType(k).isEmpty())
            mars += qMakePair(k, QRegExp(re));
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
