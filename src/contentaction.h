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

#ifndef CONTENTACTION_H
#define CONTENTACTION_H

/*!
  \mainpage Libcontentaction

  \brief Libcontentaction is a library for associating uri's to the
  set of applicable ContentAction objects and triggering the actions.

  \section Overview

  Libcontentaction reads the Nepomuk classes of the uri's from
  Tracker. For each class, there is a set of applicable actions. Each
  action is a maemo service framework interface + function name pair.

  Libcontentaction can retrive the set of applicable actions and a
  default action for a given set of uris, and return the ContentAction
  objects representing those actions. A ContentAction object can
  be used for triggering the corresponding action.

  For now, the association rules between Nepomuk classes and
  applicable actions are hard-coded.

  \section future Future plans

  The applicable service framework functions should have a unified
  interface: they should have exactly one parameter: the list of
  uri's. This way libcontentaction could trigger the needed actions
  without containing hard-coded logic about how to call each service
  framework function.

  Also, the association between Nepomuk classes and applicable actions
  can be made configurable.

*/

/*!
  \class ContentAction

  \brief ContentAction represents an applicable action for a uri or a
  set of uri's.

  The ContentAction object contains two parts: the uri or a set of
  uri's it refers to, and an action, represented by maemo service
  framework interface + function.

  The applicable ContentAction objects for a given uri or a set of
  uri's can be retrieved by using the static member functions
  actions(const QString& uri) and actions(const QStringList&
  uris). The return value is a list of ContentAction objects sorted in
  the order of relevance.

  Each Nepomuk class is associated with a set of applicable
  actions. The set of applicable actions for one uri is computed by
  concatenating the action lists for all its classes.

  For multiple uri's, the set of applicable actions is the
  intersection of applicable actions for each uri. The actions appear
  in the same order as they appear in the action list of the first
  uri.

  The ContentAction can be triggered by using the member function
  ContentAction::trigger().

  It is also possible to retrive the default actions for a given uri
  or a set of uris via member functions defaultAction(const QString& uri)
  and defaultAction(const QStringList& uris).

  For now, the default action is always the most relevant action (also
  the first action) returned by the actions() function.

  When the list of uri's contains uri's of different classes, no
  default action can be constructed, and the defaultAction function
  returns an invalid ContentAction.

*/
#include <QList>
#include <QString>
#include <QStringList>

namespace ContentAction
{

struct ActionPrivate;

class Action
{
public:
    void trigger() const;

    void setAsDefault();
    bool isDefault() const;
    bool canBeDefault() const;

    bool isValid() const;
    QString name() const;

    static Action defaultAction(const QString& uri);
    static Action defaultAction(const QStringList& uris);

    static QList<Action> actions(const QString& uri);
    static QList<Action> actions(const QStringList& uris);

    Action();
    Action(const Action& other);
    ~Action();
    Action& operator=(const Action& other);

private:
    Action(const QStringList& uris, const QStringList& classes,
                  const QString& action);

    ActionPrivate* d; /// Pimpl pointer
};

struct ActionPrivate
{
    bool valid; ///< whether or not the action can be triggered
    QStringList uris; ///< the target uri's of the action
    QStringList classes; ///< the classes of the uri's (if they are of the same type)
    QString action; ///< [service fw interface].[method]
};

}

#endif
