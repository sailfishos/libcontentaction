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
#include "contentaction-dui.h"

#include <QRegExp>
#include <QGraphicsGridLayout>
#include <QDebug>
#include <QStringListModel>
#include <QModelIndex>
#include <QAbstractListModel>

#include <DuiLabel>
#include <DuiLabelHighlighter>
#include <DuiPopupList>

Q_DECLARE_METATYPE(ContentAction::Action);

namespace {

using ContentAction::Action;

class LCALabelHighlighter: public DuiCommonLabelHighlighter
{
    Q_OBJECT
public:
    LCALabelHighlighter(const QRegExp& regexp,
                        const QStringList& actions,
                        QObject *parent = 0);
private slots:
    void doDefaultAction(const QString& match);
    void doPopupActions(const QString& match);
    void doAction(const QModelIndex& ix);
private:
    const QStringList& actions;
};

LCALabelHighlighter::LCALabelHighlighter(const QRegExp& regexp,
                                         const QStringList& actions,
                                         QObject *parent) :
    DuiCommonLabelHighlighter(regexp),
    actions(actions)
{
    if (parent)
        setParent(parent);
    QObject::connect(this, SIGNAL(clicked(const QString&)),
                     this, SLOT(doDefaultAction(const QString&)));
    QObject::connect(this, SIGNAL(longPressed(const QString&)),
                     this, SLOT(doPopupActions(const QString&)));
}

void LCALabelHighlighter::doDefaultAction(const QString& match)
{
    Action defAction = ContentAction::Internal::highlightAction(match,
                                                                actions[0]);
    defAction.trigger();
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
                ret = QString("!!%1").arg(actions.at(ix.row()).name());
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
    foreach (const QString& a, actions) {
        alist << ContentAction::Internal::highlightAction(match, a);
    }
    DuiPopupList *popuplist = new DuiPopupList();
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

/// Attaches possibly several DuiLabelHighlighter:s to the label, based on the
/// highlighter configuration.  Clicking on a highlighted label invokes the
/// first action defined for the matching pattern.  A long-click causes a
/// popup list to be shown with the possible actions, from where the user may
/// choose one.
void ContentAction::Dui::highlightLabel(DuiLabel *label)
{
    using namespace ContentAction::Internal;

    const HighlighterMap& cfg = highlighterConfig();
    QHashIterator<QString, QStringList> iter(cfg);
    while (iter.hasNext()) {
        iter.next();
        LCALabelHighlighter *hl = new LCALabelHighlighter(QRegExp(iter.key()),
                                                          iter.value(),
                                                          label);
        label->addHighlighter(hl);
    }
}

#include "dui-highlight.moc"
