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

#include "contentaction.h"
#include "internal.h"
#include "service.h"

#include <DuiDesktopEntry>
#include <QDebug>

/*!
  \class ContentAction::Action

  \brief ContentAction::Action represents an applicable action for a uri or a
  set of uri's.

  The ContentAction::Action object contains two parts: the uri or a set of
  uri's it refers to, and an action, represented by maemo service framework
  interface + function.

  The applicable ContentAction::Action objects for a given uri or a set of
  uri's can be retrieved by using the static member functions actions(const
  QString& uri) and actions(const QStringList& uris). The return value is a
  list of ContentAction::Action objects sorted in the order of relevance.

  Each Nepomuk class is associated with a set of applicable actions. The list
  of applicable actions for one uri is computed by concatenating the action
  lists for all its classes and then sorting by their weights.

  For multiple uri's, the set of applicable actions is the intersection of
  applicable actions for each uri. The actions appear in the same order as
  they appear in the action list of the first uri.

  The ContentAction::Action can be triggered by using the member function
  trigger().

  It is also possible to retrive the default actions for a given uri or a set
  of uris via member functions defaultAction(const QString& uri) and
  defaultAction(const QStringList& uris).

  When the list of uri's contains uri's of different classes, no default
  action can be constructed, and the defaultAction function returns an invalid
  ContentAction::Action.

*/
namespace ContentAction {

using namespace ContentAction::Internal;

namespace Internal {
const QString XMaemoServiceKey("Desktop Entry/X-Maemo-Service");
const QString XOssoServiceKey("Desktop Entry/X-Osso-Service");
const QString XMaemoMethodKey("Desktop Entry/X-Maemo-Method");
const QString XMaemoObjectPathKey("Desktop Entry/X-Maemo-Object-Path");
const QString ExecKey("Desktop Entry/Exec");
}

ActionPrivate::~ActionPrivate()
{
}

void ActionPrivate::setAsDefault()
{
    LCA_WARNING << "called setAsDefault() on an invalid action";
}

bool ActionPrivate::isDefault() const
{
    return false;
}

bool ActionPrivate::canBeDefault() const
{
    return false;
}

bool ActionPrivate::isValid() const
{
    return false;
}

QString ActionPrivate::name() const
{
    return QString("Invalid action");
}

QString ActionPrivate::localizedName() const
{
    return name();
}

QString ActionPrivate::icon() const
{
    LCA_WARNING << "called icon() for something that doesn't implement it";
    return "NOT_IMPLEMENTED";
}

void ActionPrivate::trigger() const
{
    LCA_WARNING << "triggered an invalid action, not doing anything.";
}

DefaultPrivate::DefaultPrivate(DuiDesktopEntry* desktopEntry, const QStringList& params)
    : desktopEntry(desktopEntry), params(params)
{
}

DefaultPrivate::~DefaultPrivate()
{
    delete desktopEntry;
}

bool DefaultPrivate::isValid() const
{
    return true;
}

QString DefaultPrivate::name() const
{
    return desktopEntry->fileName();
}

QString DefaultPrivate::localizedName() const
{
    return desktopEntry->name();
}

QString DefaultPrivate::icon() const
{
    return desktopEntry->icon();
}

Action::Action()
    : d(new ActionPrivate())
{
}

Action::Action(ActionPrivate* priv)
    : d(priv)
{
}

Action createAction(const QString& desktopFile, const QStringList& params)
{
    DuiDesktopEntry* desktopEntry = new DuiDesktopEntry(desktopFile);
    if (desktopEntry->contains(XMaemoMethodKey) &&
        !desktopEntry->contains(XMaemoServiceKey)) {
        return Action(new ServiceFwPrivate(desktopEntry, params));
    }
    else if (desktopEntry->contains(XMaemoServiceKey) ||
             desktopEntry->contains(XOssoServiceKey)) {
        return Action(new DBusPrivate(desktopEntry, params));
    }
    else if (desktopEntry->contains(ExecKey)) {
        return Action(new ExecPrivate(desktopEntry, params));
    }
    else {
        // We don't know how to launch
        return Action(new ActionPrivate());
    }
}

Action::~Action()
{
}

/// Triggers the action represented by this Action object,
/// using the uri's contained by the Action object.
void Action::trigger() const
{
    d->trigger();
}

/// Sets the action represented by this Action to be the default for a
/// Nepomuk class. If there is only one uri, the class for which the
/// default is set is the lowest class in the class hierarchy. If
/// there are multiple uri's, the default action can be set only if
/// they represent objects of the same type. In this case, the Nepomuk
/// class is decided the same way as in the case of one uri.
void Action::setAsDefault()
{
    d->setAsDefault();
}

/// Returns true if the current Action object is the default
/// action for the set of uri's it refers to.
bool Action::isDefault() const
{
    return d->isDefault();
}

/// Semantics TBD.
bool Action::canBeDefault() const
{
    return d->canBeDefault();
}

/// Returns true if the Action object represents an action
/// which can be triggered.
bool Action::isValid() const
{
    return d->isValid();
}

/// Returns the name of the action, i.e., [service fw interface].[function]
QString Action::name() const
{
    return d->name();
}

/// Returns the localized name of the action, using the currently installed
/// translators.
QString Action::localizedName() const
{
    Internal::initializeLocales();
    return d->localizedName();
}

/// Returns the icon name for the action.
QString Action::icon() const
{
    return d->icon();
}

} // end namespace
