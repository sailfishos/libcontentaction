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

  \brief Libcontentaction is a library for associating uri's to the set of
  applicable ContentAction::Action objects and triggering the actions.

  \section Overview

  Libcontentaction reads the Nepomuk classes of the uri's from Tracker. For
  each class, there is a set of applicable actions. Each action is represented
  by a "<Maemo service framework interface>.<method>" string.

  Libcontentaction can retrive the set of applicable actions and a default
  action for a given set of uris, and return the ContentAction::Action objects
  representing those actions. An ContentAction::Action object can be used for
  triggering the corresponding action.

  The association rules between Nepomuk classes and applicable actions are
  specified via .xml files in `/etc/contentaction/actions.d' (this location
  might be overridden using the $CONTENTACTION_ACTIONS environment variable).
  The format is illustrated by the next example:

  \code
  <?xml version="1.0"?>
  <actions>
    <class name="http://www.tracker-project.org/temp/nmm#MusicPiece">
      <action name="com.example.musicplayer.play"
              weight="100"/>
    </class>
    <class name="http://www.semanticdesktop.org/ontologies/2007/03/22/nfo#Image">
      <action name="com.example.pictureviewer.showImage"
              weight="100"/>
      <action name="your.service.print" weight="200"/>
    </class>
  </actions>
  \endcode

  \section Free-text highlighter

  Passing a string to highlight() can be used to discover interesting parts of
  the text.  It returns Match objects identifying the position of the match
  and the possible actions.

  \note

  These actions have different semantics than ordinary Actions.  When
  triggered, they call the service method with a single string argument
  containing the matched text (as UTF-8).  These are very likely _not_ valid
  Tracker URIs (subjects)!

  \subsection Configuring highlighting

  Service implementors who wish to provide new `highlightable' parts should
  drop a .xml file in the `/etc/contentaction/actions.d' directory.  The
  schema is similar to the example above, except that instead of <class>
  elements, one should specify <highlight> elements, containing the regexp to
  match for, and the applicable actions as their children.  Example:

  \code
  <?xml version="1.0"?>
  <actions>
    <highlight regexp="[^ @]+@([^ .]+)(\.[^ .]+)*">
      <action name="com.example.mua.composeMailTo"/>
    </highlight>
    <highlight regexp="\+?\d+([- ]\d+)*">
      <action name="com.example.phone.call"/>
      <action name="com.example.addressbook.add"/>
    </highlight>
  </actions>
  \endcode

  \section future Future plans

  The applicable service framework functions should have a unified
  interface: they should have exactly one parameter: the list of
  uri's. This way libcontentaction could trigger the needed actions
  without containing hard-coded logic about how to call each service
  framework function.

  Localization of action names is still under discussion.

*/

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
#include <QList>
#include <QString>
#include <QStringList>

namespace ContentAction
{

struct Match;

class Action
{
public:
    void setAsDefault();
    bool isDefault() const;
    bool canBeDefault() const;

    bool isValid() const;
    QString name() const;

    static Action defaultAction(const QString& uri);
    static Action defaultAction(const QStringList& uris);

    static QList<Action> actions(const QString& uri);
    static QList<Action> actions(const QStringList& uris);

    static QList<Match> highlight(const QString& text);

    struct DefaultPrivate;
    struct TrackerPrivate;
    struct HighlightPrivate;

    Action(DefaultPrivate *priv);
    Action();
    Action(const Action& other);
    ~Action();
    Action& operator=(const Action& other);

public slots:
    void trigger() const;

private:
    static Action trackerAction(const QStringList& uris,
                                const QStringList& classes,
                                const QString& action);
    static Action highlightAction(const QString& text,
                                  const QString& action);
    DefaultPrivate* d; /// Pimpl pointer

    friend QList<Match> highlight(const QString&);
};

struct Action::DefaultPrivate {
    virtual ~DefaultPrivate();
    virtual void setAsDefault();
    virtual bool isDefault() const;
    virtual bool canBeDefault() const;
    virtual bool isValid() const;
    virtual QString name() const;
    virtual void trigger() const;
    virtual DefaultPrivate *clone() const;
};

struct Action::TrackerPrivate: Action::DefaultPrivate {
    TrackerPrivate(const QStringList& uris,
                   const QStringList& classes,
                   const QString& action);
    virtual ~TrackerPrivate();
    virtual void setAsDefault();
    virtual bool isDefault() const;
    virtual bool canBeDefault() const;
    virtual bool isValid() const;
    virtual QString name() const;
    virtual void trigger() const;
    virtual DefaultPrivate *clone() const;

    QStringList uris; ///< the target uri's of the action
    QStringList classes; ///< the classes of the uri's (if they are of the
                         ///< same type)
    QString action; ///< [service fw interface].[method]
};

struct Action::HighlightPrivate: Action::DefaultPrivate {
    HighlightPrivate(const QString& match, const QString& action);
    virtual ~HighlightPrivate();
    virtual bool isValid() const;
    virtual QString name() const;
    virtual void trigger() const;
    virtual DefaultPrivate *clone() const;

    QString match;
    QString action;
};

struct Match {
    QList<Action> actions; ///< list of applicable actions
    int start, end; ///< [start, end) determines the matching substring

    bool operator<(const Match& other) const;
};

}

#endif
