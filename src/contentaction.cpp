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

void ContentAction::trigger()
{
    // A big switch with d->service
    if (d->service == "com.nokia.galleryserviceinterface.showImage") {
        if (gallery == 0) gallery = new galleryinterface();
        gallery->showImage(d->uri, QStringList());
    }
    // etc.
}

void ContentAction::setAsDefault()
{
}

bool ContentAction::isDefault()
{
    return false;
}

bool ContentAction::canBeDefault()
{
    return false;
}

ContentAction ContentAction::defaultAction(const QString& uri)
{
    return defaultAction(QStringList() << uri);
}

ContentAction ContentAction::defaultAction(const QStringList& uris)
{
    return ContentAction();
}

QList<ContentAction> ContentAction::actions(const QString& uri)
{
    return QList<ContentAction>();
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

QStringList actionsForClass(const QString& klass)
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
