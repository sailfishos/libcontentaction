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
#include "internal.h"

#include <stdlib.h>

#include <QDebug>
#include <QRegularExpression>
#include <QDir>
#include <QStringList>
#include <QXmlStreamReader>

const QString ContentAction::HighlighterMimeClass("x-maemo-highlight/");

namespace {

using namespace ContentAction;
using namespace ContentAction::Internal;

// mime type -> regexp
static QList<QPair<QString, QRegularExpression> > Highlighter_cfg;
// raw data for Highlighter_cfg
static QHash<QString, QString> mimeToRegexp;
static QHash<QString, QString> mimeToParent;

/// Returns the path where the action configuration files should be read from.
/// It may be overridden via the $CONTENTACTION_ACTIONS environment variable.
static QString actionPath()
{
    const char *path = ::getenv("CONTENTACTION_ACTIONS");
    if (!path)
        path = DEFAULT_ACTIONS;
    return QString(path);
}

struct ConfigReader
{
    QString error;

    bool parse(QIODevice *device)
    {
        QXmlStreamReader xml(device);

        while (!xml.atEnd()) {
            xml.readNext();

            if (xml.isStartElement()) {
                if (xml.name() == "actions") {
                    continue;
                } else if (xml.name() == "highlight") {
                    parseHighlight(xml);
                } else {
                    error = "unexpected tag: " + xml.name().toString();
                    return false;
                }
            }
        }

        if (xml.hasError()) {
            error = xml.errorString();
            return false;
        }

        return true;
    }

    void parseHighlight(QXmlStreamReader &xml)
    {
        auto atts = xml.attributes();

        QString regexp = atts.value("regexp").toString();
        if (regexp.isEmpty()) {
            error = "expected a nonempty regexp";
            return;
        }

        QString mime = atts.value("name").toString().trimmed();
        if (mime.isEmpty()) {
            error = "expected a nonempty mimetype";
            return;
        }

        mimeToRegexp.insert(mime, regexp);

        QString parent = atts.value("specialCaseOf").toString();
        if (!parent.isEmpty()) {
            mimeToParent.insert(mime, parent);
        }
    }
};

// Constructs Highlighter_cfg from mimeToRegexp and mimeToParent.
// Sorts the regexps topologically so that the special cases appear before the general cases.
static void sortRegexps()
{
    // Insert the regexps in the wrong order (parent first, parent is the more
    // general regexp).  But always prepend, so the list will be in the right
    // order (special case before the general case).
    QString toInsert;
    QString original;

    while (!mimeToRegexp.isEmpty()) {
        // Take any regexp
        toInsert = mimeToRegexp.begin().key();
        original = toInsert; // store the starting point (for loop detection)
        while (mimeToParent.contains(toInsert)
               && mimeToRegexp.contains(mimeToParent.value(toInsert))) {
            // There is a parent and parent not yet inserted
            toInsert = mimeToParent.value(toInsert);

            if (toInsert == original) {
                LCA_WARNING << "Loop in regexp specialization:" << toInsert;
                // a loop
                break;
            }
        }
        // Insert, and also remove from mimeToRegexp to note it has been inserted
        QString rule = mimeToRegexp.take(toInsert);
        QRegularExpression expression(rule);

        if (expression.isValid()) {
            Highlighter_cfg.prepend(qMakePair(QString(HighlighterMimeClass) + toInsert, expression));
        } else {
            qWarning() << "Invalid highlight rule:" << rule << "-- " << expression.errorString();
        }
    }
}

static void readConfig()
{
    static bool read = false;

    if (read)
        return;
    read = true;

    QDir dir(actionPath());
    if (!dir.isReadable()) {
        LCA_WARNING << "cannot read actions from" << dir.path();
        return;
    }
    dir.setNameFilters(QStringList("*.xml"));
    QStringList confFiles = dir.entryList(QDir::Files);
    Q_FOREACH (const QString& confFile, confFiles) {
        QFile file(dir.filePath(confFile));

        ConfigReader handler;

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            LCA_WARNING << "cannot open" << file.fileName();
            continue;
        }

        if (!handler.parse(&file)) {
            LCA_WARNING << "failed to parse" << file.fileName() << ":" << handler.error;
            continue;
        }
    }

    // Sort the regexps topologically: each regexp (e.g., a specialized url)
    // before its parent (e.g., a more general url)
    sortRegexps();
    mimeToRegexp.clear();
    mimeToParent.clear();
}

} // end anon namespace

/// Returns the highlighter configuration map of (mimetype, regexp) read from
/// the configuration files.
const QList<QPair<QString, QRegularExpression> >& ContentAction::Internal::highlighterConfig()
{
    readConfig();
    return Highlighter_cfg;
}
