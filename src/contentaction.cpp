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

#include <libtracker-client/tracker.h>
#include <galleryinterface.h>

#include <QDebug>

// initialized on the first request
static TrackerClient *Tracker = 0;
static galleryinterface *gallery = 0;

ContentAction::ContentAction()
{
    d = new ContentActionPrivate();
    d->valid = false;
}

ContentAction::ContentAction(const QString& uri, const QStringList& classes,
                             const QString& action)
{
    d = new ContentActionPrivate();
    d->uri = uri;
    d->classes = classes;
    d->action = action;
    d->valid = true;
}

ContentAction::ContentAction(const ContentAction& other)
{
    d = new ContentActionPrivate();
    *d = *other.d;
}

ContentAction::~ContentAction()
{
    delete d;
    d = 0;
}

ContentAction& ContentAction::operator=(const ContentAction& other)
{
    *d = *other.d;
}

void ContentAction::trigger() const
{
    if (!d->valid) {
        qWarning() << "triggered an invalid action, not doing anything.";
        return;
    }

    if (d->action == "com.nokia.galleryserviceinterface.showImage") {
        if (gallery == 0)
            gallery = new galleryinterface("foo.bar");
        if (gallery->isValid())
            gallery->showImage(d->uri, QStringList());
        else
            qWarning() << "galleryinterface is invalid";
    }
}

void ContentAction::setAsDefault()
{
    if (!d->valid) {
        qWarning() << "called setAsDefault() on an invalid action";
        return;
    }
}

bool ContentAction::isDefault() const
{
    if (!d->valid)
        return false;
    return false;
}

bool ContentAction::canBeDefault() const
{
    if (!d->valid)
        return false;
    return false;
}

ContentAction ContentAction::defaultAction(const QString& uri)
{
    QList<ContentAction> acts = actions(uri);
    if (acts.isEmpty())
        return ContentAction();
    else
        return acts[0];
}

ContentAction ContentAction::defaultAction(const QStringList& uris)
{
    return ContentAction();
}

QList<ContentAction> ContentAction::actions(const QString& uri)
{
    QList<ContentAction> result;
    QStringList classes = classesOf(uri);
    foreach (const QString& klass, classes) {
        QStringList actions = actionsForClass(klass);
        foreach (const QString& action, actions) {
            result << ContentAction(uri, classes, action);
        }
    }
    return result;
}

QList<ContentAction> ContentAction::actions(const QStringList& uris)
{
    // E.g. call actions(uri) several times and take the intersection
    return QList<ContentAction>();
}

static bool isValidIRI(const QString& uri)
{
    static const QRegExp validRE("[^<>\"{}|^`\\\\]*");
    return validRE.exactMatch(uri);
}

QStringList ContentAction::classesOf(const QString& uri)
{
    QStringList result;

    if (!isValidIRI(uri)) {
        qWarning() << "invalid characters in uri:" << uri;
        return result;
    }
    if (!Tracker) {
        Tracker = tracker_connect(TRUE, 0);
        if (!Tracker) {
            qWarning() << "failed to connect to Tracker";
            return result;
        }
    }
    QString query = QString("SELECT ?s WHERE { <%1> a ?s . }").arg(uri);
    GError *error = NULL;
    GPtrArray *resArray = tracker_resources_sparql_query(Tracker,
                                                         query.toLocal8Bit().data(),
                                                         &error);
    if (error) {
        qWarning() << "query returned an error:" << error->message;
        g_error_free(error);
        return result;
    }
    for (guint i = 0; i < resArray->len; ++i) {
        char **row = (char **)g_ptr_array_index(resArray, i);
        // NOTE: we assume Tracker returns utf8
        result << QString::fromUtf8(row[0]);
        g_strfreev(row);
    }
    g_ptr_array_free(resArray, TRUE);
    return result;
}

QStringList ContentAction::actionsForClass(const QString& klass)
{
    // Hard-coded association between nepomuk classes and actions (=
    // service fw interface + method)
    QStringList result;

    if (klass.endsWith("nmm#MusicPiece")) {
        result << "";
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
