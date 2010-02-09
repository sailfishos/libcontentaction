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

#include <QDebug>

namespace ContentAction {

using namespace ContentAction::Internal;

ServiceResolver resolver;

Action::DefaultPrivate::~DefaultPrivate() { }
void Action::DefaultPrivate::setAsDefault()
{
    LCA_WARNING << "called setAsDefault() on an invalid action";
}
bool Action::DefaultPrivate::isDefault() const
{
    return false;
}
bool Action::DefaultPrivate::canBeDefault() const
{
    return false;
}
bool Action::DefaultPrivate::isValid() const
{
    return false;
}
QString Action::DefaultPrivate::name() const
{
    return QString("Invalid action");
}
void Action::DefaultPrivate::trigger() const
{
    LCA_WARNING << "triggered an invalid action, not doing anything.";
}
Action::DefaultPrivate *Action::DefaultPrivate::clone() const
{
    return new DefaultPrivate();
}

Action::Action() : d(new DefaultPrivate())
{ }

Action::Action(DefaultPrivate *priv) : d(priv)
{ }

Action::Action(const Action& other)
{
    d = other.d->clone();
}

Action::~Action()
{
    delete d;
    d = 0;
}

Action& Action::operator=(const Action& other)
{
    DefaultPrivate *ourd = d;
    DefaultPrivate *np = other.d->clone();
    d = np;
    delete ourd;
    return *this;
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

} // end namespace
