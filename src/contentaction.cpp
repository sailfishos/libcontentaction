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
#include <QCoreApplication>
#include <QTranslator>
#include <QStringList>
#include <QByteArray>

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

Action::DefaultPrivate::~DefaultPrivate()
{ }

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

QString Action::DefaultPrivate::localizedName() const
{
    return name();
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

/// Installs (or re-installs) QCoreApplication translators based on \a locale.
/// Call it at the beginning of the program, or when locale changes.  See also
/// the \ref Localization section.
///
/// The default search directory might be overridden with the
/// CONTENTACTION_L10N_DIR environment variable.
void Action::installTranslators(const QString& locale)
{
    static QList<QTranslator *> cur_translators;
    static QString l10ndir;

    if (l10ndir.isEmpty()) {
        QByteArray dir = qgetenv("CONTENTACTION_L10N_DIR");
        if (!dir.isEmpty())
            l10ndir = QString::fromLocal8Bit(dir);
        else
            l10ndir = QString::fromLocal8Bit(DEFAULT_L10N_DIR);
    }

    while (!cur_translators.isEmpty()) {
        QTranslator *tr = cur_translators.takeFirst();
        QCoreApplication::removeTranslator(tr);
        delete tr;
    }
    foreach (const QString& qmfn, translationsConfig()) {
        QTranslator *tr = new QTranslator();
        QString fullfn = qmfn + "_" + locale;
        if (!tr->load(fullfn, l10ndir))
            LCA_WARNING << "failed to load translation:" << fullfn;
        cur_translators << tr;
        QCoreApplication::installTranslator(tr);
    }
}

/// Returns the localized name of the action, using the currently installed
/// translators.
QString Action::localizedName() const
{
    return d->localizedName();
}

} // end namespace
