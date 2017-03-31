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
bool Internal::mimeAndUriFromTracker(const QStringList& uris, QStringList &urlsAndMimes)
{
    QString query("SELECT ");
    Q_FOREACH (const QString& uri, uris)
        query += QString("nie:url(<%1>) nie:mimeType(<%1>) ").arg(uri);
    query += " {}";
    QDBusReply<QVector<QStringList> > reply = tracker()->call(SparqlQuery, query);
    if (!reply.isValid())
        return false;
    urlsAndMimes = reply.value()[0];
    Q_FOREACH (const QString& x, urlsAndMimes)
        if (x.isEmpty()) return false;
    return true;
}

// A special hack for WebHistory and Bookmark. We check the nfo:bookmarks and
// nfo:uri properties, and if the uri has either of them defined, we check the
// object of that property, and dispatch it by its scheme.
static bool hactionSchemeFromTracker(const QStringList& uris, QStringList &urls)
{
    QString query("SELECT ");
    Q_FOREACH (const QString& uri, uris)
        query += QString("tracker:coalesce(nfo:bookmarks(<%1>), nfo:uri(<%1>)) ").arg(uri);
    query += " {}";
    QDBusReply<QVector<QStringList> > reply = tracker()->call(SparqlQuery, query);
    if (!reply.isValid())
        return false;
    urls = reply.value()[0];
    Q_FOREACH (const QString& x, urls)
        if (x.isEmpty()) return false;
    return true;
}

// Another special hack, this time for mfo:Enclosure:s.  We follow
// their mfo:localLink property and dispatch again on that.
static bool hactionFileFromTracker(const QString& uri, QString &path, QString &mimeType)
{
    QString query = ("SELECT "
                     + QString("mfo:localLink(<%1>) nie:mimeType(<%1>)").arg(uri)
                     + " {}");
    QDBusReply<QVector<QStringList> > reply = tracker()->call(SparqlQuery, query);
    if (!reply.isValid())
        return false;
    path = reply.value()[0][0];
    mimeType = reply.value()[0][1];
    if (path.isEmpty())
      return false;
    else
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
    Q_FOREACH (const QString& mimeType, conditions.keys()) {
        // Don't consider mime types for which nobody defines an action,
        // except if it is `software-application' which is special case.
        QString pseudoMimeType(OntologyMimeClass + mimeType);

        if (checkTrackerCondition(conditions[mimeType], uri))
            mimeTypes << pseudoMimeType;
    }
    return mimeTypes;
}

// Returns a list of per-uri pseudo-mimetypes applicable to each of \a uris.
// [["mime1-for-uri1", "mime2-for-uri1"], ["mime1-for-uri2"], ...]
// It would be amazing if there would be only one pseudo mime-type per uri.
static QList<QStringList> mimeTypesForUris(const QStringList& uris)
{
    QList<QStringList> allMimeTypes;
    Q_FOREACH (const QString& uri, uris) {
        if (!isValidIRI(uri)) return QList<QStringList>();
        allMimeTypes << mimeForTrackerObject(uri);
    }
    return allMimeTypes;
}

/// Returns the default action for the given \a uri representing an object
/// stored in Tracker.  A default action is determined by checking the \ref
/// tracker_conditions "conditions" that apply to the \a uri, and taking the
/// first one having a default action.  If no default action is found in this
/// way, it checks whether \a uri represents a resource with \c nie:url and \c
/// nie:mimeType properties.  If there is a default action for the given
/// mimetype, it is returned (note that triggering this action will pass the
/// \c nie:url as argument).  If neither methods succeeded, it falls back to
/// the most relevant action, i.e. the first action returned by actions().  If
/// there are no applicable actions, an invalid Action object is returned.
Action Action::defaultAction(const QString& uri)
{
    if (!isValidIRI(uri)) return Action();
    QStringList mimeTypes = mimeForTrackerObject(uri);
    LCA_DEBUG << "pseudo-mimes" << mimeTypes;
    Q_FOREACH (const QString& mimeType, mimeTypes) {
        QString def = findDesktopFile(defaultAppForContentType(mimeType));
        if (!def.isEmpty())
            return createAction(def,
                                QStringList() << uri);
    }
    // If the resource is a file-based one, query its url and mimetype, and
    // consider the default action for them.
    QStringList urlAndMime;
    if (mimeAndUriFromTracker(QStringList() << uri, urlAndMime)) {
        LCA_DEBUG << "real url and mimetype" << urlAndMime;
        // Tracker has the filename %-encoded, don't encode it again
        QUrl fileUrl = QUrl::fromEncoded(urlAndMime[0].toUtf8());
        return defaultActionForFile(fileUrl, urlAndMime[1]);
    }

    // FIXME: this is a hack for converting nfo:Bookmark and nfo:WebHistory into
    // a url.
    QStringList hackUrl;
    if (hactionSchemeFromTracker(QStringList() << uri, hackUrl)) {
        LCA_DEBUG << "hack url" << hackUrl;
        return defaultActionForScheme(hackUrl[0]);
    }

    // FIXME, too
    QString hackPath, hackMimeType;
    if (hactionFileFromTracker(uri, hackPath, hackMimeType)) {
        LCA_DEBUG << "hack file" << hackPath << hackMimeType;
        return defaultActionForFile(hackPath, hackMimeType);
    }

    // Fall back to one of the existing actions (if there are some)
    LCA_DEBUG << "fallback to actions()";
    QList<Action> acts = actions(uri);
    if (acts.size() >= 1)
        return acts[0];
    return Action();
}

/// Returns the default action for a given list of \a uris representing
/// Tracker resources.  If the URIs represent objects of different types
/// (e.g. one is an image, other is an audio file), a default action cannot be
/// constructed in this manner.  In that case the \c nie:mimeType properties
/// of all \a uris are checked and we attempt to construct a default action
/// based on them (passing \c nie:url as argument) (\see
/// Action::defaultAction(const QString& uri) also).  If both ways fail, it
/// falls back to the first action returned by actions().  If there are none,
/// an invalid Action is returned.
Action Action::defaultAction(const QStringList& uris)
{
    if (uris.isEmpty())
        return Action();

    // FIXME: this function doesn't include the hack for nfo:Bookmark &
    // nfo:WebHistory.

    QList<QStringList> mimeTypes = mimeTypesForUris(uris);
    LCA_DEBUG << "pseudo mimes per uri" << mimeTypes;
    QSet<QString> defApps;
    for (int i = 0; i < mimeTypes.size(); ++i) {
        QSet<QString> defs;
        Q_FOREACH (const QString& mimeType, mimeTypes[i]) {
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
    Q_FOREACH (const QString& appid, defApps) {
        QString app = findDesktopFile(appid);
        if (!app.isEmpty())
            return createAction(app, uris);
    }

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
        QString app = findDesktopFile(defApp);
        if (app.isEmpty())
            return createAction(app, fileUris);
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
/// is one of the mime types of the uri, and construct the Action:s
/// accordingly.  Additionally we check whether the \a uri has both \c nie:url
/// and \c nie:mimeType properties, in which case we append extra Actions for
/// the corresponding mime types.  Note that these actions are passed the \c
/// nie:url as argument.
QList<Action> Action::actions(const QString& uri)
{
    QList<Action> result;
    if (!isValidIRI(uri)) return result;

    QStringList mimeTypes = mimeForTrackerObject(uri);
    LCA_DEBUG << "pseudo mimes" << mimeTypes;
    QSet<QString> blackList; // for adding each action only once
    Q_FOREACH (const QString& mimeType, mimeTypes) {
        QStringList apps = appsForContentType(mimeType);
        Q_FOREACH (const QString& appid, apps) {
            QString app = findDesktopFile(appid);
            if (!app.isEmpty()) {
                result << createAction(app,
                                       QStringList() << uri);
                blackList.insert(result.last().name());
            }
        }
    }
    // Construct additional actions based on nie:mimeType(uri), passing
    // nie:url(uri) as argument. However, don't add actions for those
    // applications which already are in the list (because they declare handling
    // the x-maemo-nepomuk mime type).
    QStringList urlAndMime;
    if (mimeAndUriFromTracker(QStringList() << uri, urlAndMime)) {
        LCA_DEBUG << "real url and mimetype" << urlAndMime;
        // Tracker has the filename %-encoded, don't encode it again
        QUrl fileUrl = QUrl::fromEncoded(urlAndMime[0].toUtf8());
        QList<Action> actions = actionsForFile(fileUrl, urlAndMime[1]);
        Q_FOREACH (const Action& a, actions) {
            if (!blackList.contains(a.name())) {
                result << a;
            }
        }
    }
    // And still others, if it happens to be a nfo:Bookmark or nfo:WebHistory.
    // FIXME: this is a hack for converting nfo:Bookmark and nfo:WebHistory into
    // a url.
    QStringList hackUrl;
    if (hactionSchemeFromTracker(QStringList() << uri, hackUrl)) {
        LCA_DEBUG << "hack url" << hackUrl;
        result << actionsForScheme(hackUrl[0]);
    }

    // FIXME, too
    // FIXME, too
    QString hackPath, hackMimeType;
    if (hactionFileFromTracker(uri, hackPath, hackMimeType)) {
        LCA_DEBUG << "hack file" << hackPath << hackMimeType;
        return actionsForFile(hackPath, hackMimeType);
    }

    // TODO: sort the result
    return result;
}

/// Returns the set of actions applicable to all \a uris (which represent
/// Tracker resources).  The set is constructed by first figuring out the
/// "pseudo-mimetypes" for all \a uris.  The resulting actions are the common
/// ones handling all \a uris.  The order of the actions is the order in which
/// they appear in the action list of the first uri.  Additional actions for
/// \c nie:mimeType properties of the \a uris are appended similarly to the
/// single-uri version of this function.  \see Action::actions(const QString&
/// uri) for details.
QList<Action> Action::actions(const QStringList& uris)
{
    QList<Action> result;

    if (uris.isEmpty())
        return result;

    QList<QStringList> mimeTypes = mimeTypesForUris(uris);

    // FIXME: this function doesn't include the hack for nfo:WebHistory.

    LCA_DEBUG << "pseudo mimes per uri" << mimeTypes;
    QStringList commonApps;
    for (int i = 0; i < mimeTypes.size(); ++i) {
        QStringList apps;
        Q_FOREACH (const QString& mime, mimeTypes[i])
            apps += appsForContentType(mime);
        if (i == 0) {
            commonApps = apps;
        } else {
            QStringList intersection;
            Q_FOREACH (const QString& commonApp, commonApps) {
                if (apps.contains(commonApp))
                    intersection << commonApp;
            }
            commonApps = intersection;
        }
    }
    LCA_DEBUG << "commonApps" << commonApps;
    QSet<QString> blackList; // for adding each action only once
    Q_FOREACH (const QString& appid, commonApps) {
        QString app = findDesktopFile(appid);
        if (!app.isEmpty()) {
            result << createAction(app, uris);
            blackList.insert(result.last().name());
        }
    }

    QStringList urlsAndMimes;
    if (mimeAndUriFromTracker(uris, urlsAndMimes)) {
        LCA_DEBUG << "real urls and mimes" << urlsAndMimes;
        commonApps.clear();
        QStringList fileUris;
        for (int i = 0; i < urlsAndMimes.size(); i += 2) {
            fileUris << urlsAndMimes[i];
            QStringList apps = appsForContentType(urlsAndMimes[i+1]);
            if (i == 0) {
                commonApps = apps;
            } else {
                QStringList intersection;
                Q_FOREACH (const QString& commonApp, commonApps) {
                    if (apps.contains(commonApp))
                        intersection << commonApp;
                }
                commonApps = intersection;
            }
        }
        LCA_DEBUG << "real-mime commonApps" << commonApps;
        Q_FOREACH (const QString& appid, commonApps) {
            QString app = findDesktopFile(appid);
            if (app.isEmpty())
                continue;
            Action a = createAction(app, fileUris);
            if (!blackList.contains(a.name())) {
                result << a;
            }
        }
    }
    // TODO: sort the result
    return result;
}

} // end namespace
