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

#include <QDebug>

// initialized on the first request
static TrackerClient *Tracker = 0;
static galleryinterface *gallery = 0;
static MusicSuiteServicePublicIf *musicSuite = 0;

#define LCA_WARNING qWarning() << "libcontentaction:"

ContentAction::ContentAction()
{
    d = new ContentActionPrivate();
    d->valid = false;
}

ContentAction::ContentAction(const QStringList& uris, const QStringList& classes,
                             const QString& action)
{
    d = new ContentActionPrivate();
    d->uris = uris;
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
    return *this;
}

void ContentAction::trigger() const
{
    if (!d->valid) {
        LCA_WARNING << "triggered an invalid action, not doing anything.";
        return;
    }

    if (d->action == "com.nokia.galleryserviceinterface.showImage") {
        if (gallery == 0)
            gallery = new galleryinterface();
        if (!gallery->isValid()) {
            LCA_WARNING << "gallery interface is invalid";
            return;
        }
        gallery->showImage("", d->uris);
    }
    else if (d->action == "com.nokia.MusicSuiteServicePublicIf.play") {
        if (musicSuite == 0)
            musicSuite = new MusicSuiteServicePublicIf();
        if (!musicSuite->isValid()) {
            LCA_WARNING << "music suite interface is invalid";
            return;
        }
        foreach (const QString& uri, d->uris)
            musicSuite->play(uri);
    }
}

void ContentAction::setAsDefault()
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

    // If the action concerns multiple uris, but they are not of the
    // same type, this action cannot be set as a default action.
    if (d->classes.isEmpty()) {
        return false;
    }

    // For now, all actions are applicable as default actions.
    return true;
}

bool ContentAction::isValid() const
{
    return d->valid;
}

QString ContentAction::name() const
{
    return d->action;
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
    /// XXX: is there always a default action? Is the most relevant
    /// action always the default action?
    QList<ContentAction> acts = actions(uris);
    if (acts.isEmpty())
        return ContentAction();
    else
        return acts[0];
}

/// Returns the set of applicable actions for a given \a uri. The nepomuk
/// classes of the uri are read from Tracker, and the actions are
/// determined with hard-coded association rules between nepomuk
/// classes and actions.
QList<ContentAction> ContentAction::actions(const QString& uri)
{
    QList<ContentAction> result;
    QStringList classes = classesOf(uri);
    foreach (const QString& klass, classes) {
        QStringList actions = actionsForClass(klass);
        foreach (const QString& action, actions) {
            result << ContentAction(QStringList() << uri, classes, action);
        }
    }
    return result;
}

/// Returns the set of actions applicable to all \a uris. The set is
/// an intersection of actions applicable to the individual uris.
QList<ContentAction> ContentAction::actions(const QStringList& uris)
{
    QStringList commonActions;
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

    QList<ContentAction> result;
    foreach (const QString& act, commonActions)
        result << ContentAction(uris, commonClasses, act);
    return result;
}

static bool isValidIRI(const QString& uri)
{
    static const QRegExp validRE("[^<>\"{}|^`\\\\]*");
    return validRE.exactMatch(uri);
}

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
    QSet<QString> done;
    QSet<QString> currentLevel = classes.keys().toSet() - supers;
    QSet<QString> nextLevel;
    while (currentLevel.size() > 0) {
        foreach(const QString& c, currentLevel) {
            if (!done.contains(c)) {
                done << c;
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

QStringList actionsForClass(const QString& klass)
{
    // Hard-coded association between nepomuk classes and actions (=
    // service fw interface + method)
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
    QString defAction = defaultActionForClass(klass);
    if (result.contains(defAction)) {
        result.removeAll(defAction);
        result.prepend(defAction);
    }
    return result;
}

/// Reads the per-class default action from GConf
QString defaultActionForClass(const QString& klass)
{
    return "";
}
