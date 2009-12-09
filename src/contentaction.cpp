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

#include <libtracker-client/tracker.h>
#include <galleryinterface.h>
#include <musicsuiteservicepublicif.h>

#include <gconf/gconf.h>
#include <gconf/gconf-client.h>
#include <QDebug>

namespace ContentAction {

// initialized on the first request
static TrackerClient *Tracker = 0;
static GConfClient *Gconf;
static galleryinterface *Gallery = 0;
static MusicSuiteServicePublicIf *MusicSuite = 0;

#define LCA_WARNING qWarning() << "libcontentaction:"
#define GCONF_KEY_PREFIX "/apps/contentaction/"

Action::Action()
{
    d = new ActionPrivate();
    d->valid = false;
}

Action::Action(const QStringList& uris, const QStringList& classes,
                             const QString& action)
{
    d = new ActionPrivate();
    d->uris = uris;
    d->classes = classes;
    d->action = action;
    d->valid = true;
}

Action::Action(const Action& other)
{
    d = new ActionPrivate();
    *d = *other.d;
}

Action::~Action()
{
    delete d;
    d = 0;
}

Action& Action::operator=(const Action& other)
{
    *d = *other.d;
    return *this;
}

/// Triggers the action represented by this Action object,
/// using the uri's contained by the Action object.
void Action::trigger() const
{
    if (!d->valid) {
        LCA_WARNING << "triggered an invalid action, not doing anything.";
        return;
    }

    if (d->action == "com.nokia.galleryserviceinterface.showImage") {
        if (Gallery == 0)
            Gallery = new galleryinterface();
        if (!Gallery->isValid()) {
            LCA_WARNING << "gallery interface is invalid";
            return;
        }
        Gallery->showImage("", d->uris);
    }
    else if (d->action == "com.nokia.MusicSuiteServicePublicIf.play") {
        if (MusicSuite == 0)
            MusicSuite = new MusicSuiteServicePublicIf();
        if (!MusicSuite->isValid()) {
            LCA_WARNING << "music suite interface is invalid";
            return;
        }
        foreach (const QString& uri, d->uris)
            MusicSuite->play(uri);
    }
}

/// Sets the action represented by this Action to be the default for a
/// Nepomuk class. If there is only one uri, the class for which the
/// default is set is the lowest class in the class hierarchy. If
/// there are multiple uri's, the default action can be set only if
/// they represent objects of the same type. In this case, the Nepomuk
/// class is decided the same way as in the case of one uri. Note: Not
/// yet implemented.
void Action::setAsDefault()
{
    if (!d->valid) {
        LCA_WARNING << "called setAsDefault() on an invalid action";
        return;
    }
    // If the action concerns multiple uris, but they are not of the
    // same type, we cannot set a default action.
    if (d->classes.isEmpty()) {
        LCA_WARNING << "cannot set a default action for multiple uris of different types";
        return;
    }
    // Set this action to be the default action for the most specific
    // class
    setDefaultAction(d->classes[0], d->action);
}

/// Returns true if the current Action object is the default
/// action for the set of uri's it refers to. Note: not yet implemented.
bool Action::isDefault() const
{
    if (!d->valid)
        return false;

    Action def = defaultAction(d->uris);
    return (def.d->action == d->action);
}

/// Semantics TBD.
bool Action::canBeDefault() const
{
    if (!d->valid)
        return false;

    // If the action concerns multiple uris, but they are not of the
    // same type, this action cannot be set as a default action.
    if (d->classes.isEmpty()) {
        return false;
    }

    // For now, all actions are applicable as default actions.
    return true;
}

/// Returns true if the Action object represents an action
/// which can be triggered.
bool Action::isValid() const
{
    return d->valid;
}

/// Returns the name of the action, i.e., [service fw interface].[function]
QString Action::name() const
{
    return d->action;
}

/// Returns the default action for a given uri. A default action is
/// determined by walking up the class hierarchy of the \a uri, and
/// taking the first default action defined for a class. If no default
/// action is set, returns the most relevant action, i.e., the first
/// action returned by actions(). If there are no applicable actions,
/// an invalid Action object is returned.
Action Action::defaultAction(const QString& uri)
{
    // Walk through the class list of the uri, if we find a default
    // action for a class, return it and stop.
    QStringList classes = classesOf(uri);
    QString action = defaultActionForClasses(classes);
    if (action != "") {
        return Action(QStringList() << uri, classes, action);
    }
    // No default actions were found from GConf. Fall back to the most
    // relevant action.
    foreach (const QString& klass, classes) {
        QStringList actions = actionsForClass(klass);
        if (actions.size() > 0)
            return Action(QStringList() << uri, classes, actions[0]);
    }
    return Action();
}

/// Returns the default action for a given list of uri's. If the uri's
/// represent object of different types (e.g., one is an image, other
/// is an audio file), a default action cannot be constructed and an
/// invalid Action object is returned.
Action Action::defaultAction(const QStringList& uris)
{
    bool first = true;
    QStringList commonClasses;
    foreach (const QString& uri, uris) {
        QStringList classes = classesOf(uri);

        if (first) {
            commonClasses = classes;
            first = false;
        } else {
            if (classes != commonClasses) {
                commonClasses.clear();
                break;
            }
        }
    }
    QString action = defaultActionForClasses(commonClasses);
    if (action != "") {
        return Action(uris, commonClasses, action);
    }
    // No default actions were found from GConf. Fall back to the most
    // relevant action.
    foreach (const QString& klass, commonClasses) {
        QStringList actions = actionsForClass(klass);
        if (actions.size() > 0)
            return Action(uris, commonClasses, actions[0]);
    }
    return Action();
}

/// Returns the set of applicable actions for a given \a uri. The nepomuk
/// classes of the uri are read from Tracker, and the actions are
/// determined with hard-coded association rules between nepomuk
/// classes and actions.
QList<Action> Action::actions(const QString& uri)
{
    QList<Action> result;
    QStringList classes = classesOf(uri);
    foreach (const QString& klass, classes) {
        QStringList actions = actionsForClass(klass);
        foreach (const QString& action, actions) {
            result << Action(QStringList() << uri, classes, action);
        }
    }
    return result;
}

/// Returns the set of actions applicable to all \a uris. The set is
/// an intersection of actions applicable to the individual uris.
QList<Action> Action::actions(const QStringList& uris)
{
    QStringList commonActions;
    // Empty list if the uri's are not of the same type; otherwise the
    // class list of them.
    QStringList commonClasses;
    bool first = true;

    foreach (const QString& uri, uris) {
        QStringList classes = classesOf(uri);
        QStringList acts;

        foreach (const QString& klass, classes)
            acts.append(actionsForClass(klass));

        if (first) {
            commonClasses = classes;
            commonActions = acts;
            first = false;
        } else {
            if (classes != commonClasses)
                commonClasses.clear();
            // Remove all actions not applicable to this uri.
            QStringList intersection;
            foreach (const QString& act, commonActions) {
                if (acts.contains(act))
                    intersection << act;
            }
            commonActions = intersection;
        }
    }

    QList<Action> result;
    foreach (const QString& act, commonActions)
        result << Action(uris, commonClasses, act);
    return result;
}

/// Returns true if the \a uri is a valid uri.
static bool isValidIRI(const QString& uri)
{
    static const QRegExp validRE("[^<>\"{}|^`\\\\]*");
    return validRE.exactMatch(uri);
}

/// Returns the Nepomuk classes of a given \a uri. The classes are
/// returned in the following order: first the immediate superclasses
/// of the uri in arbitrary order, then their superclasses in
/// arbitrary order, etc.
QStringList classesOf(const QString& uri)
{
    QStringList result;

    if (!isValidIRI(uri)) {
        LCA_WARNING << "invalid characters in uri:" << uri;
        return result;
    }
    if (!Tracker) {
        Tracker = tracker_connect(TRUE, 0);
        if (!Tracker) {
            LCA_WARNING << "failed to connect to Tracker";
            return result;
        }
    }
    QString query = QString("SELECT ?sub ?super WHERE {<%1> a ?sub . "
                            "OPTIONAL {?sub rdfs:subClassOf ?super . "
                            "OPTIONAL {?between rdfs:subClassOf ?super . "
                            "?sub rdfs:subClassOf ?between .} "
                            "FILTER(! bound(?between)) } }").arg(uri);
    // In the result, sub is a class of uri and super is an immediate
    // superclass of sub.

    GError *error = NULL;
    GPtrArray *resArray = tracker_resources_sparql_query(Tracker,
                                                         query.toLocal8Bit().data(),
                                                         &error);
    if (error) {
        LCA_WARNING << "query returned an error:" << error->message;
        g_error_free(error);
        return result;
    }

    QHash<QString, QStringList> classes; // class -> its immediate superclasses
    QSet<QString> supers; // classes which have at least one subclass
    for (guint i = 0; i < resArray->len; ++i) {
        char **row = (char **)g_ptr_array_index(resArray, i);
        // NOTE: we assume Tracker returns utf8
        QString sub = QString::fromUtf8(row[0]);
        QString super = QString::fromUtf8(row[1]);

        if (!classes.contains(sub))
            classes.insert(sub, QStringList());
        if (super != "")
            classes[sub] << super;

        supers << super;

        g_strfreev(row);
    }
    g_ptr_array_free(resArray, TRUE);

    // Starting from the most immediate classes of the uri, iterate
    // the superclasses according to superclass levels. (The most
    // immediate first, then all their superclasses, etc.)
    QSet<QString> currentLevel = classes.keys().toSet() - supers;
    QSet<QString> nextLevel;
    while (currentLevel.size() > 0) {
        foreach(const QString& c, currentLevel) {
            // Note: result.contains(c) is inefficient, but likely the
            // number of classes is very small.
            if (!result.contains(c)) {
                result << c;
                foreach(const QString& super, classes[c])
                    nextLevel << super;
            }
        }
        currentLevel = nextLevel;
        nextLevel.clear();
    }
    return result;
}

/// Returns the hard-coded list of action names for the given Nepomuk
/// class \a klass.
QStringList actionsForClass(const QString& klass)
{
    QStringList result;

    if (klass.endsWith("nmm#MusicPiece")) {
        result << "com.nokia.MusicSuiteServicePublicIf.play";
    }
    else if (klass.endsWith("nmm#MusicAlbum")) {
        result << "";
    }
    else if (klass.endsWith("nmm#Video")) {
        result << "";
    }
    else if (klass.endsWith("nmm#Playlist")) {
        result << "";
    }
    else if (klass.endsWith("nmm#ImageList")) {
        result << "";
    }
    else if (klass.endsWith("nfo#Image")) {
        result << "com.nokia.galleryserviceinterface.showImage";
    }
    else if (klass.endsWith("nfo#Audio")) {
        result << "";
    }
    return result;
}

/// Returns the per-class default action. If there is no default
/// action for that class, returns an empty string.
QString defaultAction(const QString& klass)
{
    if (Gconf == 0) {
        g_type_init(); // XXX: needed?
        Gconf = gconf_client_get_default();
    }

    // Query the value from GConf
    char* escaped = gconf_escape_key(klass.toLocal8Bit().constData(), -1);
    QString key = QString(GCONF_KEY_PREFIX) + QString::fromAscii(escaped);

    GError* error = NULL;
    GConfValue* value = gconf_client_get(Gconf, key.toLocal8Bit().constData(), &error);

    g_free(escaped);

    if (error) {
        LCA_WARNING << "Error getting data from GConf:" << error->message;
        g_error_free(error);
        return "";
    }

    if (value == 0) {
        // The key doesn't exist; no default action for the class
        return "";
    }

    const char* valueString = gconf_value_get_string(value); // shouldn't be freed

    QString action = "";
    if (valueString) {
        action = QString(valueString);
    }

    gconf_value_free(value);
    return action;
}

bool setDefaultAction(const QString& klass, const QString& action)
{
    if (Gconf == 0) {
        g_type_init(); // XXX: needed?
        Gconf = gconf_client_get_default();
    }

    // Set the class - action pair to GConf
    char* escaped = gconf_escape_key(klass.toLocal8Bit().constData(), -1);
    QString key = QString(GCONF_KEY_PREFIX) + QString::fromAscii(escaped);

    GError* error = NULL;

    gconf_client_set_string(Gconf, key.toLocal8Bit().constData(), action.toLocal8Bit().constData(), &error);
    g_free(escaped);

    if (error) {
        LCA_WARNING << "Error setting data to GConf:" << error->message;
        g_error_free(error);
        return false;
    }
    return true;
}

/// Walks up the inheritance hierarchy, checking the default actions
/// for each class. Returns the first action found, or an empty string
/// if none were found.
QString defaultActionForClasses(const QStringList& classes)
{
    foreach (const QString& klass, classes) {
        QString action = defaultAction(klass);
        if (action != "") {
            return action;
        }
    }
    return "";
}

} // end namespace
