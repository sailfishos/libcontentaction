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

#include <libtracker-client/tracker-client.h>

#include <QDir>
#include <QFile>
#include <QPair>
#include <QHash>
#include <QDBusInterface>
#include <QDebug>
#include <QCoreApplication>

namespace ContentAction {

using namespace ContentAction::Internal;

const QString OntologyMimeClass("x-maemo-nepomuk/");

// initialized on the first request
static TrackerClient *Tracker = 0;

/// Returns true if the \a uri is a valid uri.
static bool isValidIRI(const QString& uri)
{
    static const QRegExp validRE("[^<>\"{}|^`\\\\]*");
    return validRE.exactMatch(uri);
}

static bool checkTrackerCondition(const QString& condition, const QString& uri)
{
    if (!isValidIRI(uri)) {
        LCA_WARNING << "invalid characters in uri:" << uri;
        return false;
    }
    if (!Tracker) {
        Tracker = tracker_client_new(TRACKER_CLIENT_ENABLE_WARNINGS, 0);
        if (!Tracker) {
            LCA_WARNING << "failed to connect to Tracker";
            return false;
        }
    }

    QString query = QString("SELECT 1 { %1 FILTER (?uri = <%2>)}")
        .arg(condition).arg(uri);

    GError *error = NULL;
    GPtrArray *res = tracker_resources_sparql_query(Tracker,
                                                    query.toUtf8().data(),
                                                    &error);
    if (error) {
        LCA_WARNING << "query returned an error:" << error->message;
        g_error_free(error);
        return false;
    }

    if (res->len != 0 && res->len != 1) {
        LCA_WARNING << "unexpected number of rows returned:" << res->len;
    }

    bool toReturn = (res->len > 0);
    if (res->len > 0) {
        char **row = (char **)g_ptr_array_index(res, 0);
        g_strfreev(row);
    }
    g_ptr_array_free(res, TRUE);
    return toReturn;
}

/// Evaluates the specified tracker conditions and check which match the \a
/// uri.
QStringList Internal::mimeForTrackerObject(const QString& uri)
{
    QStringList mimeTypes;
    QHash<QString, QString> conditions = trackerConditions();
    foreach (const QString& mimeType, conditions.keys()) {
        // Don't consider mime types for which nobody defines an action
        QStringList apps = appsForContentType(OntologyMimeClass + mimeType);
        if (apps.isEmpty()) continue;

        if (checkTrackerCondition(conditions[mimeType], uri)) {
            mimeTypes << (OntologyMimeClass + mimeType);
        }
    }
    return mimeTypes;
}

/// Evaluates the specified tracker conditions and check which match all URIS
/// in \a uris.
static QStringList mimeTypesForUris(QStringList uris)
{
    bool first = true;
    QStringList commonMimeTypes;
    foreach (const QString& uri, uris) {
        QStringList mimeTypes = mimeForTrackerObject(uri);

        if (first) {
            commonMimeTypes = mimeTypes;
            first = false;
        } else {
            // Remove the mime types which don't apply to uri
            QStringList intersection;
            foreach (const QString& mimeType, commonMimeTypes)
                if (mimeTypes.contains(mimeType))
                    intersection << mimeType;
            commonMimeTypes = intersection;
        }
    }
    return commonMimeTypes;
}

/// Returns the default action for the given \a uri representing an object
/// stored in Tracker.  A default action is determined by checking the \ref
/// tracker_conditions "conditions" that apply to the \a uri, and taking the
/// first one having a default action.  If no default action is found, it
/// returns the most relevant action, i.e. the first action returned by
/// actions().  If there are no applicable actions, an invalid Action object
/// is returned.
Action Action::defaultAction(const QString& uri)
{
    QStringList mimeTypes = mimeForTrackerObject(uri);
    foreach (const QString& mimeType, mimeTypes) {
        QString def = defaultAppForContentType(mimeType);
        if (!def.isEmpty())
            return createAction(findDesktopFile(def),
                                QStringList() << uri);
    }
    // Fall back to one of the existing actions (if there are some)
    QList<Action> acts = actions(uri);
    if (acts.size() >= 1)
        return acts[0];
    return Action();
}

/// Returns the default action for a given list of URIs.  If the URIs
/// represent objects of different types (e.g. one is an image, other is an
/// audio file), a default action cannot be constructed and an invalid Action
/// object is returned.
Action Action::defaultAction(const QStringList& uris)
{
    QStringList mimeTypes = mimeTypesForUris(uris);
    foreach (const QString& mimeType, mimeTypes) {
        QString def = defaultAppForContentType(mimeType);
        if (!def.isEmpty())
            return createAction(findDesktopFile(def),
                                uris);
    }
    // Fall back to one of the existing actions (if there are some)
    QList<Action> acts = actions(uris);
    if (acts.size() >= 1)
        return acts[0];
    return Action();
}

/// Returns the set of applicable actions for an object stored in Tracker
/// represented by the given \a uri.  The applicability is determined by the
/// pre-defined \ref tracker_conditions "Tracker conditions".  If a condition
/// \c cond applies to \a uri, then we consider that \c x-maemo-nepomuk/cond1
/// is one of the mime types of the uri, and search the applicable
/// applications accordingly (and alliterating :).
QList<Action> Action::actions(const QString& uri)
{
    QList<Action> result;

    QStringList mimeTypes = mimeForTrackerObject(uri);
    foreach (const QString& mimeType, mimeTypes) {
        QStringList apps = appsForContentType(mimeType);
        foreach (const QString& app, apps) {
            result << createAction(findDesktopFile(app),
                                   QStringList() << uri);
        }
    }
    // TODO: sort the result
    return result;
}

/// Returns the set of actions applicable to all \a uris.  The set is
/// constructed by first figuring out the common "pseudo-mimetypes" for all \a
/// uris.  For each mimetype, we add the actions handling it.  The order of
/// the actions is the order in which they appear in the action list of the
/// first uri.
QList<Action> Action::actions(const QStringList& uris)
{
    QList<Action> result;
    QStringList mimeTypes = mimeTypesForUris(uris);

    foreach (const QString& mimeType, mimeTypes) {
        QStringList apps = appsForContentType(mimeType);
        foreach (const QString& app, apps) {
            result << createAction(findDesktopFile(app),
                                   uris);
        }
    }
    // TODO: sort the result
    return result;
}

} // end namespace
