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

#include <MLabel>
#include <MLabelHighlighter>
#include <MPopupList>

namespace {

using namespace ContentAction;
using namespace ContentAction::Internal;

class LCALabelHighlighter: public MCommonLabelHighlighter
{
    Q_OBJECT
public:
    LCALabelHighlighter(const QRegExp& regexp,
                        const QString& mime,
                        QObject *parent = 0);
private slots:
    void doDefaultAction(const QString& match);
    void doPopupActions(const QString& match);
    void doAction(const QModelIndex& ix);
private:
    QString mime;
};

LCALabelHighlighter::LCALabelHighlighter(const QRegExp& regexp,
                                         const QString& mime,
                                         QObject *parent) :
    MCommonLabelHighlighter(regexp),
    mime(mime)
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
    QString app = defaultAppForContentType(mime);
    Action defAction = createAction(findDesktopFile(app), QStringList() << match);
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
    QStringList apps = appsForContentType(mime);
    foreach (const QString& app, apps) {
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

static void hiLabel(MLabel *label,
                    const QHash<QString, QString>& cfg)
{
    QHashIterator<QString, QString> iter(cfg);
    QObjectList hiliters;

    if (label->property("_lca_highlighters").isValid())
        return;
    while (iter.hasNext()) {
        iter.next();
        // iter.key == mime type, iter.value == regexp
        if (!appsForContentType(iter.key()).isEmpty() &&
            !defaultAppForContentType(iter.key()).isEmpty()) {
            LCALabelHighlighter *hl = new LCALabelHighlighter(QRegExp(iter.value()),
                                                              iter.key(),
                                                              label);
            hiliters.append(hl);
            label->addHighlighter(hl);
        }
    }
    label->setProperty("_lca_highlighters", QVariant::fromValue(hiliters));
}

static void unhiliteLabel(MLabel *label)
{
    QVariant prop = label->property("_lca_highlighters");
    if (!prop.isValid())
        return;
    QObjectList hiliters = prop.value<QObjectList>();
    foreach (QObject *hl, hiliters) {
        LCALabelHighlighter *lcahl = static_cast<LCALabelHighlighter *>(hl);
        label->removeHighlighter(lcahl);
        delete lcahl;
    }
    label->setProperty("_lca_highlighters", QVariant());
}

/// Attaches possibly several MLabelHighlighter:s to the label, based on the
/// highlighter configuration.  Clicking on a highlighted label invokes the
/// first action defined for the matching pattern.  A long-click causes a
/// popup list to be shown with the possible actions, from where the user may
/// choose one.
void ContentAction::highlightLabel(MLabel *label)
{
    const QHash<QString, QString>& cfg = highlighterConfig();
    hiLabel(label, cfg);
}

/// Similar to highlightLabel() but allows specifying which regexp-types to
/// highlight (e.g. only \c "x-maemo-highlight/mailto").
void ContentAction::highlightLabel(MLabel *label,
                                        QStringList typesToHighlight)
{
    const QHash<QString, QString>& cfg = highlighterConfig();
    QHash<QString, QString> filtered;
    foreach (const QString& k, typesToHighlight) {
        QString re(cfg.value(k, QString()));
        if (re.isEmpty())
            continue;
        filtered.insert(k, re);
    }
    hiLabel(label, filtered);
}

/// Removes all highlighters attached by highlightLabel() from \a label.
void ContentAction::dehighlightLabel(MLabel *label)
{
    unhiliteLabel(label);
}

void ContentAction::Dui::highlightLabel(DuiLabel *label)
{
    LCA_WARNING << "DuiLabel support is removed, migrate to MeeGoTouch.";
}

/// Similar to highlightLabel() but allows specifying which regexp-types to
/// highlight (e.g. only \c "x-maemo-highlight/mailto").
void ContentAction::Dui::highlightLabel(DuiLabel *label,
                                        QStringList typesToHighlight)
{
    LCA_WARNING << "DuiLabel support is removed, migrate to MeeGoTouch.";
}

/// Removes all highlighters attached by highlightLabel() from \a label.
void ContentAction::Dui::dehighlightLabel(DuiLabel *label)
{
    LCA_WARNING << "DuiLabel support is removed, migrate to MeeGoTouch.";
}

Q_DECLARE_METATYPE(QObjectList);
Q_DECLARE_METATYPE(ContentAction::Action);

#include "highlight.moc"
