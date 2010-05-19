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

#include <QDir>
#include <QFile>
#include <QPair>
#include <QHash>
#include <QStringList>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusMetaType>
#include <QDebug>
#include <QCoreApplication>

Q_DECLARE_METATYPE(QVector<QStringList>)

#define LCA_DEBUG                                   \
    if (1) {} else (qDebug() << (__func__) << ':')

namespace ContentAction {
using namespace ContentAction::Internal;

const QString OntologyMimeClass("x-maemo-nepomuk/");
static const QString SoftwareApplicationMimeType("x-maemo-nepomuk/software-application");
static const QString SparqlQuery("SparqlQuery");

// Returns true if the \a uri is a valid IRI, warns otherwise.
static bool isValidIRI(const QString& uri)
{
    static const QRegExp validRE("[^<>\"{}|^`\\\\]*");
    if (!validRE.exactMatch(uri)) {
        LCA_WARNING << "invalid characters in uri:" << uri;
        return false;
    } else
        return true;
}

static QDBusInterface *tracker()
{
    static QDBusInterface* Tracker = 0;
    if (!Tracker) {
        qDBusRegisterMetaType<QVector<QStringList> >();
        Tracker = new QDBusInterface(QLatin1String("org.freedesktop.Tracker1"),
                                     QLatin1String("/org/freedesktop/Tracker1/Resources"),
                                     QLatin1String("org.freedesktop.Tracker1.Resources"));
    }
    return Tracker;
}

// Queries nie:url and nie:mimeType properties for the given \a uris.  If the
// query didn't fail, urlsAndMimes is set to the list of ["url1", "mime1",
// "url2", "mime2", ...].  Finally it checks that all urls and mimes are
// nonempty and returns true if this holds.
static bool mimeAndUriFromTracker(const QStringList& uris, QStringList &urlsAndMimes)
{
    QString query("SELECT ");
    foreach (const QString& uri, uris)
        query += QString("nie:url(<%1>) nie:mimeType(<%1>) ").arg(uri);
    query += " {}";
    QDBusReply<QVector<QStringList> > reply = tracker()->call(SparqlQuery, query);
    if (!reply.isValid())
        return false;
    urlsAndMimes = reply.value()[0];
    foreach (const QString& x, urlsAndMimes)
        if (x.isEmpty()) return false;
    return true;
}

// Queries tracker whether \a condition applies to \a uri.
static bool checkTrackerCondition(const QString& condition, const QString& uri)
{
    QString query = QString("SELECT 1 { %1 FILTER (?uri = <%2>)}")
        .arg(condition).arg(uri);

    QDBusReply<QVector<QStringList> > reply = tracker()->call(SparqlQuery, query);

    if (!reply.isValid() || reply.value().size() == 0)
        return false;
    return true;
}

/// Evaluates all tracker conditions and checks which apply to the given \a
/// uri.  Returns a list of pseudo-mimetypes.
QStringList Internal::mimeForTrackerObject(const QString& uri)
{
    QStringList mimeTypes;
    QHash<QString, QString> conditions = trackerConditions();
    // TODO: evaluate them at once.
    foreach (const QString& mimeType, conditions.keys()) {
        // Don't consider mime types for which nobody defines an action,
        // except if it is `software-application' which is special case.
        QString pseudoMimeType(OntologyMimeClass + mimeType);
        if (pseudoMimeType != SoftwareApplicationMimeType &&
            appsForContentType(pseudoMimeType).isEmpty()) continue;

        if (checkTrackerCondition(conditions[mimeType], uri))
            mimeTypes << pseudoMimeType;
    }
    return mimeTypes;
}

// Returns a list of pseudo-mimetypes of tracker conditions per uri applicable
// to all of the given \a uris.
static QList<QStringList> mimeTypesForUris(const QStringList& uris)
{
    QList<QStringList> allMimeTypes;
    foreach (const QString& uri, uris) {
        if (!isValidIRI(uri)) return QList<QStringList>();
        allMimeTypes << mimeForTrackerObject(uri);
    }
    return allMimeTypes;
}

// Given a nfo:SoftwareApplication uri constructs an Action, which when
// triggered launches the corresponding application.
static Action createSoftwareAction(const QString& uri)
{
    QString query("SELECT nie:url(<%1>) {}");
    QDBusReply<QVector<QStringList> > reply = tracker()->call(SparqlQuery, query.arg(uri));
    if (!reply.isValid())
        return Action();
    QString fileUri(reply.value()[0][0]);
    if (fileUri.isEmpty())
        return Action();
    QUrl desktopFileUri(fileUri);
    return createAction(desktopFileUri.toLocalFile(), QStringList());
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
    if (!isValidIRI(uri)) return Action();
    QStringList mimeTypes = mimeForTrackerObject(uri);
    LCA_DEBUG << "pseudo-mimes" << mimeTypes;
    foreach (const QString& mimeType, mimeTypes) {
        if (mimeType == SoftwareApplicationMimeType)
            return createSoftwareAction(uri);
        QString def = defaultAppForContentType(mimeType);
        if (!def.isEmpty())
            return createAction(findDesktopFile(def),
                                QStringList() << uri);
    }
    // If the resource is a file-based one, query its url and mimetype, and
    // consider the default action for them.
    QStringList urlAndMime;
    if (mimeAndUriFromTracker(QStringList() << uri, urlAndMime)) {
        LCA_DEBUG << "real url and mimetype" << urlAndMime;
        return defaultActionForFile(urlAndMime[0], urlAndMime[1]);
    }
    // Fall back to one of the existing actions (if there are some)
    LCA_DEBUG << "fallback to actions()";
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
    QList<QStringList> mimeTypes = mimeTypesForUris(uris);
    LCA_DEBUG << "pseudo mimes per uri" << mimeTypes;
    QSet<QString> defApps;
    for (int i = 0; i < mimeTypes.size(); ++i) {
        QSet<QString> defs;
        foreach (const QString& mimeType, mimeTypes[i]) {
            QString def = defaultAppForContentType(mimeType);
            if (!def.isEmpty())
                defs << def;
        }
        if (i == 0)
            defApps = defs;
        else
            defApps.intersect(defs);
    }
    LCA_DEBUG << "defApps" << defApps;
    // If there are multiple possible default applications, the choice is
    // arbitrary.
    if (!defApps.empty())
        return createAction(findDesktopFile(*defApps.begin()), uris);

    // Try mimetype based handlers for real things.
    QStringList urlsAndMimes;
    if (mimeAndUriFromTracker(uris, urlsAndMimes)) {
        LCA_DEBUG << "urlsAndMimes" << urlsAndMimes;
        QStringList fileUris;
        QString defApp;
        for (int i = 0; i < urlsAndMimes.size(); i += 2) {
            fileUris << urlsAndMimes[i];
            QString def = defaultAppForContentType(urlsAndMimes[i+1]);
            if (i == 0)
                defApp = def;
            if (defApp != def) {
                defApp.clear();
                break;
            }
        }
        LCA_DEBUG << "defApp" << defApp;
        if (!defApp.isEmpty())
            return createAction(findDesktopFile(defApp), fileUris);
    }
    // Fall back to one of the existing actions (if there are some)
    LCA_DEBUG << "fallback to actions()";
    QList<Action> acts = actions(uris);
    if (acts.size() > 0)
        return acts[0];
    return Action();
}

/// Returns the set of applicable actions for an object stored in Tracker
/// represented by the given \a uri.  The applicability is determined by the
/// pre-defined \ref tracker_conditions "Tracker conditions".  If a condition
/// \c cond applies to \a uri, then we consider that \c x-maemo-nepomuk/cond
/// is one of the mime types of the uri, and search the applicable
/// applications accordingly (and alliterating :).
QList<Action> Action::actions(const QString& uri)
{
    QList<Action> result;
    if (!isValidIRI(uri)) return result;

    QStringList mimeTypes = mimeForTrackerObject(uri);
    LCA_DEBUG << "pseudo mimes" << mimeTypes;
    foreach (const QString& mimeType, mimeTypes) {
        QStringList apps = appsForContentType(mimeType);
        if (mimeType == SoftwareApplicationMimeType)
            result << createSoftwareAction(uri);
        foreach (const QString& app, apps) {
            result << createAction(findDesktopFile(app),
                                   QStringList() << uri);
        }
    }
    // Construct additional actions based on nie:mimeType(uri), passing
    // nie:url(uri) as argument.
    QStringList urlAndMime;
    if (mimeAndUriFromTracker(QStringList() << uri, urlAndMime)) {
        LCA_DEBUG << "real url and mimetype" << urlAndMime;
        result << actionsForFile(urlAndMime[0], urlAndMime[1]);
    }
    // TODO: sort the result
    return result;
}

/// Returns the set of actions applicable to all \a uris (which represent
/// Tracker resources).  The set is constructed by first figuring out the
/// common "pseudo-mimetypes" for all \a uris.  For each mimetype, we add the
/// actions handling it.  The order of the actions is the order in which they
/// appear in the action list of the first uri.
QList<Action> Action::actions(const QStringList& uris)
{
    QList<Action> result;
    QList<QStringList> mimeTypes = mimeTypesForUris(uris);

    LCA_DEBUG << "pseudo mimes per uri" << mimeTypes;
    QSet<QString> commonApps;
    for (int i = 0; i < mimeTypes.size(); ++i) {
        QSet<QString> apps;
        foreach (const QString& mime, mimeTypes[i])
            apps += appsForContentType(mime).toSet();
        if (i == 0)
            commonApps = apps;
        else
            commonApps.intersect(apps);
    }
    LCA_DEBUG << "commonApps" << commonApps;
    foreach (const QString& app, commonApps)
        result << createAction(findDesktopFile(app), uris);

    QStringList urlsAndMimes;
    if (mimeAndUriFromTracker(uris, urlsAndMimes)) {
        LCA_DEBUG << "real urls and mimes" << urlsAndMimes;
        commonApps.clear();
        QStringList fileUris;
        for (int i = 0; i < urlsAndMimes.size(); i += 2) {
            fileUris << urlsAndMimes[i];
            if (i == 0)
                commonApps = appsForContentType(urlsAndMimes[i+1]).toSet();
            else
                commonApps.intersect(appsForContentType(urlsAndMimes[i+1]).toSet());
        }
        LCA_DEBUG << "real-mime commonApps" << commonApps;
        foreach (const QString& app, commonApps)
            result << createAction(findDesktopFile(app), fileUris);
    }
    // TODO: sort the result
    return result;
}

} // end namespace
