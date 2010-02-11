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

  \section provider_nepomuk Providing actions for Tracker URI:s

  To define a new action applicable to Tracker URI:s, the following steps are
  needed:

  - You need to have a defined D-Bus interface name, and the mapping from the
    interface name to your D-Bus name needs to be known to the DUI Service
    Framework. For this, it's enough to add a "Interface: " line to your D-Bus
    .service file. You might also want to publish your D-Bus interface in the
    maemo-services package, but it's not needed for libcontentaction.

  - You need to provide a function (over D-Bus) taking in a list of strings
    (and no other parameters). When your function is called, the strings will
    be the Tracker URI:s. It is essential that the D-Bus signature of the
    function is excatly this. (You may have a return value; we'll ignore it.)

  - The mapping of one or many Nepomuk class names to your D-Bus interface +
    function name needs to be added to the libcontentaction0 package (contact
    the implementors for this). The configuration file format is still
    evolving; in the future it will be possible to install the configuration
    files separately from libcontentaction0.

  \section provider_mime Providing actions for files

  To define a new action applicable to files, the following steps are needed:

  - You need to manifest the Mime types your application handles in the
    .desktop file of your application.

  - You also need to declare your D-Bus bus name in the .desktop file.

  - Other steps are also needed but they are still undecided.

  \section highlighter Free-text highlighter

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
#include <QUrl>

struct _GAppInfo;

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
    static Action defaultActionForFile(const QUrl& fileUri);

    static QList<Action> actions(const QString& uri);
    static QList<Action> actions(const QStringList& uris);
    static QList<Action> actionsForFile(const QUrl& fileUri);

    static QList<Match> highlight(const QString& text);

    struct DefaultPrivate;

    Action(DefaultPrivate *priv);
    Action();
    Action(const Action& other);
    ~Action();
    Action& operator=(const Action& other);

public slots:
    void trigger() const;

private:
    DefaultPrivate* d; /// Pimpl pointer

    // TODO: get rid of this
    friend class TrackerPrivate;
};

struct Match {
    QList<Action> actions; ///< list of applicable actions
    int start, end; ///< [start, end) determines the matching substring

    bool operator<(const Match& other) const;
};

} // end namespace
#endif
