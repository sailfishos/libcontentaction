/*
 * Copyright (C) 2009, 2010 Nokia Corporation.
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

#include <libtracker-client/tracker.h>

#include <gconf/gconf.h>
#include <gconf/gconf-client.h>

#include <QDir>
#include <QFile>
#include <QPair>
#include <QHash>
#include <QDBusInterface>
#include <QDebug>
#include <QCoreApplication>

namespace ContentAction {

using namespace ContentAction::Internal;

static const QString OntologyMimeClass("x-maemo-tracker/");

// initialized on the first request
static TrackerClient *Tracker = 0;
static GConfClient *Gconf = 0;

static const QString GCONF_KEY_PREFIX("/Dui/contentaction/");

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
    foreach (const QString& cls, classes) {
        defaultAppForContentType(cls);
    }
    return Action();
}

/// Returns the default action for a given list of uri's. If the uri's
/// represent object of different types (e.g., one is an image, other
/// is an audio file), a default action cannot be constructed and an
/// invalid Action object is returned.
Action Action::defaultAction(const QStringList& uris)
{
    /*bool first = true;
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
        return trackerAction(uris, commonClasses, action);
    }
    // No default actions were found from GConf. Fall back to the most
    // relevant action.
    QList<Action> acts = actions(uris);
    if (acts.size() > 0)
    return acts[0];*/
    return Action();
}

/// Returns the set of applicable actions for a given \a uri. The nepomuk
/// classes of the uri are read from Tracker.
QList<Action> Action::actions(const QString& uri)
{
    QList<Action> result;
    QStringList classes = classesOf(uri);

    foreach (const QString& klass, classes) {
        result.append(actionsForUri(uri, klass));
    }
    // TODO: sort the result
    return result;
}

/// Returns the set of actions applicable to all \a uris. The set is
/// an intersection of actions applicable to the individual uris. The
/// order of the actions is the order in which they appear in the
/// action list of the first uri.
QList<Action> Action::actions(const QStringList& uris)
{
    bool first = true;
    QStringList appsSoFar;
    foreach(const QString& uri, uris) {
        QStringList classes = classesOf(uri);
        QStringList appsForUri;
        foreach (const QString& klass, classes) {
            appsForUri.append(appsForContentType(QString("x-maemo-nepomuk/") + klass));
        }

        if (first) {
            appsSoFar = appsForUri;
        }
        else {
            // Remove the apps which cannot handle *any* of the superclasses of
            // the uri.
            QStringList intersection;
            foreach (const QString& app, appsSoFar)
                if (appsForUri.contains(app))
                    intersection << app;
            appsSoFar = intersection;
        }
    }

    QList<Action> result;
    foreach (const QString& app, appsSoFar) {
        result << createAction(findDesktopFile(app), uris);
    }
    // TODO: sort the result
    return result;
}

/// Returns true if the \a uri is a valid uri.
static bool isValidIRI(const QString& uri)
{
    static const QRegExp validRE("[^<>\"{}|^`\\\\]*");
    return validRE.exactMatch(uri);
}

// Builds a map from long ontology names => mimetypes used by the library
// ("x-maemo-tracker/prefix-Class").
static const QHash<QString, QString>& prefixMap()
{
    // long name => short name
    static QHash<QString, QString> prefixMap;
    static bool read = false;

    if (read)
        return prefixMap;

    QHashIterator<QString, QString> iter(mimeApps());
    QStringList trackerMimes;

    while (iter.hasNext()) {
        iter.next();
        if (iter.key().startsWith(OntologyMimeClass)) {
            // demangle "x-foo/prefix-Class" into "prefix:Class"
            trackerMimes << iter.key()
                .mid(OntologyMimeClass.length())
                .replace('-', ':');
        }
    }

    // handle if we don't have any mapping, in this case retry on the next
    // call
    if (trackerMimes.isEmpty())
        return prefixMap;

    QString query = QString("SELECT %1 {}").arg(trackerMimes.join(" "));
    GError *error = NULL;
    GPtrArray *res = tracker_resources_sparql_query(Tracker,
                                                    query.toUtf8().data(),
                                                    &error);
    read = true;
    if (error) {
        LCA_WARNING << "query returned an error:" << error->message;
        g_error_free(error);
        return prefixMap;
    }
    if (res->len != 1) {
        LCA_WARNING << "expected one row from the query";
        return prefixMap;
    }

    char **row = (char **)g_ptr_array_index(res, 0);
    for (int i = 0; i < trackerMimes.count(); ++i) {
        if (!row[i])
            continue;
        prefixMap.insert(QString::fromUtf8(row[i]),
                         OntologyMimeClass +
                         trackerMimes[i].replace(':', '-'));
    }
    g_strfreev(row);
    g_ptr_array_free(res, TRUE);
    return prefixMap;
}

/// Returns the mime-types corresponding to the Nepomuk classes of a given \a
/// uri.  It only returns the "semantic class path" containing
/// nie:InformationElement, and ignores the class path containing
/// nie:DataObject.  The classes are returned in the order from most immediate
/// to least immediate.
QStringList Internal::classesOf(const QString& uri)
{
    QStringList result;

    if (!isValidIRI(uri)) {
        LCA_WARNING << "invalid characters in uri:" << uri;
        return result;
    }
    if (!Tracker) {
        Tracker = tracker_client_new(TRACKER_CLIENT_ENABLE_WARNINGS, 0);
        if (!Tracker) {
            LCA_WARNING << "failed to connect to Tracker";
            return result;
        }
    }
    QString query = QString("SELECT ?sub ?super WHERE { "
                            "  <%1> a ?sub . "
                            "  OPTIONAL { ?sub rdfs:subClassOf ?super . } "
                            "}").arg(uri);
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

    // Read the "semantic class path" starting from nie:InformationElement
    QHash<QString, QStringList> classes; // class -> its immediate subclasses
    QString infoElement; // The long name for nie:InformationElement
    for (guint i = 0; i < resArray->len; ++i) {
        char **row = (char **)g_ptr_array_index(resArray, i);
        QString sub = QString::fromUtf8(row[0]);
        QString super = QString::fromUtf8(row[1]);

        if (sub.contains("InformationElement"))
            infoElement = sub;

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

    // Then reverse the list and also filter out classes for which we don't
    // have any applicable actions and map long names back to short prefixed
    // ones.
    for (int i = temp.count() - 1; i >= 0; --i) {
        if (prefixMap().contains(temp[i]))
            result << prefixMap()[temp[i]];
    }
    return result;
}

/// Returns the per-class default action. If there is no default
/// action for that class, returns an empty string.
QString Internal::defaultActionFromGConf(const QString& klass)
{
    if (Gconf == 0) {
        g_type_init(); // XXX: needed?
        Gconf = gconf_client_get_default();
    }

    // Query the value from GConf
    char* escaped = gconf_escape_key(klass.toUtf8().constData(), -1);
    QString key = GCONF_KEY_PREFIX + QString::fromAscii(escaped);

    GError* error = NULL;
    GConfValue* value = gconf_client_get(Gconf, key.toAscii().constData(), &error);

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
        action = QString::fromUtf8(valueString);
    }

    gconf_value_free(value);
    return action;
}

bool Internal::setDefaultAction(const QString& klass, const QString& action)
{
    if (Gconf == 0) {
        g_type_init(); // XXX: needed?
        Gconf = gconf_client_get_default();
    }

    // Set the class - action pair to GConf
    char* escaped = gconf_escape_key(klass.toUtf8().constData(), -1);
    QString key = GCONF_KEY_PREFIX + QString::fromAscii(escaped);

    GError* error = NULL;

    gconf_client_set_string(Gconf, key.toAscii().constData(),
                            action.toUtf8().constData(), &error);
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
QString Internal::defaultActionForClasses(const QStringList& classes)
{
    foreach (const QString& klass, classes) {
        QString action = defaultActionFromGConf(klass);
        if (action != "") {
            return action;
        }
    }
    return "";
}

} // end namespace
