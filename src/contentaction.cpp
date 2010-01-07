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
#include <QDir>
#include <QFile>
#include <QPair>
#include <QHash>
#include <QDebug>

#include <algorithm>

namespace ContentAction {

// initialized on the first request
static TrackerClient *Tracker = 0;
static GConfClient *Gconf = 0;
static galleryinterface *Gallery = 0;
static MusicSuiteServicePublicIf *MusicSuite = 0;

#define LCA_WARNING qWarning() << "libcontentaction:"
#define GCONF_KEY_PREFIX "/Dui/contentaction/"

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
    // FIXME: decide what to do if there are many "most specific" classes.
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

Action Action::defaultAction(const QUrl& uri)
{
    return defaultAction(uri.toEncoded());
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
    // TODO: implement
    QStringList classes = classesOf(uri);
    QString action = defaultActionForClasses(classes);
    if (action != "") {
        return Action(QStringList() << uri, classes, action);
    }
    // No default actions were found from GConf. Fall back to the most
    // relevant action.
    QList<Action> acts = actions(uri);
    if (acts.size() > 0)
        return acts[0];
    return Action();
}

Action Action::defaultAction(const QList<QUrl>& uris)
{
    QStringList sl;
    foreach (const QUrl& uri, uris) {
        sl << uri.toEncoded();
    }
    return defaultAction(sl);
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
    QList<Action> acts = actions(uris);
    if (acts.size() > 0)
        return acts[0];
    return Action();
}

QList<Action> Action::actions(const QUrl& uri)
{
    return actions(uri.toEncoded());
}

/// Returns the set of applicable actions for a given \a uri. The
/// nepomuk classes of the uri are read from Tracker. For each class,
/// the set of applicable actions and corresponding weights is read
/// from a configuration file.
QList<Action> Action::actions(const QString& uri)
{
    QList<Action> result;
    QList<QPair<int, QString> > allActions;
    QStringList classes = classesOf(uri);

    // Gather together the list of actions for all the classes of
    // the uri, then sort according to weight.
    foreach (const QString& klass, classes) {
        QList<QPair<int, QString> > actions = actionsForClass(klass);
        for (int i = 0; i < actions.size(); ++i)
            allActions << actions[i];
    }
    // Note: it's required that one action occurs only once in the
    // class hierarchy. E.g., it's not allowed to associate an action
    // both with nmm#Image and nmm#Photo.
    qSort(allActions);
    for (int i = 0; i < allActions.size(); ++i)
        // The order is reversed here
        result.prepend(Action(QStringList() << uri, classes, allActions[i].second));

    return result;
}

QList<Action> Action::actions(const QList<QUrl>& uris)
{
    QStringList sl;
    foreach (const QUrl& uri, uris) {
        sl << uri.toEncoded();
    }
    return actions(sl);
}

/// Returns the set of actions applicable to all \a uris. The set is
/// an intersection of actions applicable to the individual uris. The
/// order of the actions is the order in which they appear in the
/// action list of the first uri.
QList<Action> Action::actions(const QStringList& uris)
{
    QList<Action> commonActions;
    bool first = true;
    // Empty list if the uri's are not of the same type; otherwise the
    // class list of them.
    QStringList commonClasses;

    foreach (const QString& uri, uris) {
        QStringList classes = classesOf(uri);

        // Use Action::actions to get the list of applicable actions
        // for this single uri. Note that the resulting Action objects
        // have the "uri" and "classes" fields set for the individual
        // uri, so we need to correct that later. Also, the order is
        // now fixed.
        QList<Action> acts = actions(uri);
        if (first) {
            commonClasses = classes;
            commonActions = acts;
            first = false;
        } else {
            if (classes != commonClasses)
                commonClasses.clear();

            // Remove all actions not applicable to this uri.
            QSet<QString> actionNames;
            foreach (const Action& act, acts)
                actionNames.insert(act.name());

            QList<Action> intersection;
            foreach (const Action& commonAct, commonActions) {
                if (actionNames.contains(commonAct.name()))
                    intersection << commonAct;
            }
            commonActions = intersection;
        }
    }

    QList<Action> result;
    // Correct the URI's and classes of the intersected actions
    foreach (const Action& act, commonActions)
        result << Action(uris, commonClasses, act.name());

    return result;
}

/// Returns true if the \a uri is a valid uri.
static bool isValidIRI(const QString& uri)
{
    static const QRegExp validRE("[^<>\"{}|^`\\\\]*");
    return validRE.exactMatch(uri);
}

/// Returns the Nepomuk classes of a given \a uri. This only returns
/// the "semantic class path" containing nie:InformationElement, and
/// ignores the class path containing nie:DataObject. The classes are
/// returned in the order from most immediate to least immediate.
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
    // uri is expected to be percent-encoded UTF-8 here
    QString query = QString("SELECT ?sub ?super WHERE {<%1> a ?sub . "
                            "OPTIONAL {?sub rdfs:subClassOf ?super . "
                            "} }").arg(uri);
    // In the result, sub is a class of uri and super is an immediate
    // superclass of sub.

    GError *error = NULL;
    GPtrArray *resArray = tracker_resources_sparql_query(Tracker,
                                                         query.toLatin1().data(),
                                                         &error);
    if (error) {
        LCA_WARNING << "query returned an error:" << error->message;
        g_error_free(error);
        return result;
    }

    // Read the "semantic class path" starting from nie:InformationElement
    QHash<QString, QStringList> classes; // class -> its immediate subclasses
    QString infoElement = ""; // The long name for nie:InformationElement
    for (guint i = 0; i < resArray->len; ++i) {
        char **row = (char **)g_ptr_array_index(resArray, i);
        // NOTE: we assume Tracker returns utf8
        QString sub = QString::fromUtf8(row[0]);
        QString super = QString::fromUtf8(row[1]);

        if (sub.contains("InformationElement")) infoElement = sub;

        if (super != "") {
            if (!classes.contains(super))
                classes.insert(super, QStringList());
            classes[super] << sub;
        }
        g_strfreev(row);
    }
    g_ptr_array_free(resArray, TRUE);

    if (infoElement.isEmpty())
        return result;

    // Extract the classes in the order
    QStringList temp;
    temp << infoElement;
    int ix = 0;
    while (ix < temp.size())
        temp.append(classes.value(temp[ix++], QStringList()));

    // Then reverse the list; unfortunately Qt doesn't have rbegin
    std::reverse_copy(temp.begin(), temp.end(), std::back_inserter(result));
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

/// Returns the path where the action configuration files should be
/// read from.
QString actionPath()
{
    const char *path = getenv("CONTENTACTION_ACTIONS");
    if (! path)
        path = DEFAULT_ACTIONS;
    return QString(path);
}

/// Reads the configuration files for "Nepomuk class - action -
/// weight" association. Returns the list of applicable actions.
QList<QPair<int, QString> > actionsForClass(const QString& klass)
{
    static QHash<QString, QList<QPair<int, QString> > > actionsForClasses;
    static bool read = false;

    if (!read) {
        read = true;
        QString path = actionPath();
        QDir dir(path);

        if (!dir.exists() || !dir.isReadable()) {
            qWarning() << "libcontentaction: cannot read actions from" << path;
            return QList<QPair<int, QString> >();
        }

        dir.setNameFilters(QStringList("*.actions"));
        QStringList confFiles = dir.entryList(QDir::Files);
        foreach (const QString& confFile, confFiles) {
            QFile file(path + "/" + confFile);

            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                qWarning() << "Configuration file" << file.fileName() << "cannot be opened";
                continue;
            }

            QTextStream in(&file);
            while (!in.atEnd()) {
                QStringList line = in.readLine().split(" ");
                // Format of the line: class action weight
                if (line.size() < 3) {
                    qWarning() << "Too short line in configuration file" << file.fileName() << ": " << line;
                    continue;
                }
                bool conversionOk = false;
                int weight = line[2].toInt(&conversionOk);
                if (!conversionOk) {
                    qWarning() << "Invalid weight in configuration file" << file.fileName() << ": " << line[2];
                    continue;
                }
                QPair<int, QString> action(weight, line[1]);
                if (actionsForClasses.contains(line[0])) {
                    actionsForClasses[line[0]].append(action);
                }
                else {
                    actionsForClasses.insert(line[0], QList<QPair<int, QString> >() << action);
                }
            }
        }
        foreach (const QString& klass, actionsForClasses.keys())
            qSort(actionsForClasses[klass]);

    qDebug() << actionsForClasses;
    }

    if (actionsForClasses.contains(klass))
        return actionsForClasses[klass];
    return QList<QPair<int, QString> >();
}


} // end namespace
