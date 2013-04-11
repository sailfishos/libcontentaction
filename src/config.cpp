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
#include <QXmlDefaultHandler>
#include <QDir>
#include <QStringList>
#include <QMultiHash>
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
# include <QSystemDeviceInfo>
QTM_USE_NAMESPACE
#else
# include <QDeviceInfo>
# define QSystemDeviceInfo QDeviceInfo
#endif

const QString ContentAction::HighlighterMimeClass("x-maemo-highlight/");

namespace {

using namespace ContentAction;
using namespace ContentAction::Internal;

// mime type -> regexp
static QList<QPair<QString, QRegExp> > Highlighter_cfg;
// raw data for Highlighter_cfg
static QHash<QString, QString> mimeToRegexp;
static QHash<QString, QString> mimeToParent;
// friendly tracker condition name => sparql snippet
static QHash<QString, QString> Tracker_cfg;

/// Returns the path where the action configuration files should be read from.
/// It may be overridden via the $CONTENTACTION_ACTIONS environment variable.
static QString actionPath()
{
    const char *path = ::getenv("CONTENTACTION_ACTIONS");
    if (!path)
        path = DEFAULT_ACTIONS;
    return QString(path);
}

struct ConfigReader: public QXmlDefaultHandler {
    ConfigReader() : state(inLimbo) { }
    bool startElement(const QString& ns, const QString& name,
                      const QString& qname, const QXmlAttributes &atts);
    bool endElement(const QString& nsuri, const QString& name,
                    const QString& qname);
    bool characters(const QString& chars);
    QString errorString() const { return error; }
    bool fatalError(const QXmlParseException &exception) {
        LCA_WARNING << QString("parse error at line %1 column %2: %3")
            .arg(exception.lineNumber())
            .arg(exception.columnNumber())
            .arg(exception.message())
            .toLocal8Bit().constData();
        return false;
    }

    enum {
        inLimbo, inActions, inHighlight, inTrackerCondition,
    } state;

    QString condName;
    QString sparqlSnippet;
    QString error;
};

#define fail(msg)                               \
    do {                                        \
        error = msg;                            \
        return false;                           \
    } while (0)

bool ConfigReader::startElement(const QString& ns, const QString& name,
                                const QString& qname,
                                const QXmlAttributes &atts)
{
    switch (state) {
    case inLimbo:
        if (qname != "actions")
            fail("expected tag: actions");
        state = inActions;
        break;
    case inActions:
        if (qname == "highlight") {
            state = inHighlight;
            QString regexp = atts.value("regexp");
            if (regexp.isEmpty())
                fail("expected a nonempty regexp");
            QString mime = atts.value("name").trimmed();
            if (mime.isEmpty())
                fail("expected a nonempy mimetype");
            mimeToRegexp.insert(mime, regexp);
            QString parentRegexp = atts.value("specialCaseOf");
            if (!parentRegexp.isEmpty())
                mimeToParent.insert(mime, parentRegexp);
        }
        else if (qname == "tracker-condition") {
            state = inTrackerCondition;
            condName = atts.value("name");
            if (name.isEmpty())
                fail("expected a name for the condition");
            sparqlSnippet.clear();
        }
        else
            fail("unexpected tag");
        break;
    case inTrackerCondition:
    case inHighlight:
        fail("unexpected tag");
        break;
    }
    return true;
}

bool ConfigReader::characters(const QString& chars)
{
    if (state != inTrackerCondition)
        return true;
    sparqlSnippet.append(chars.trimmed());
    return true;
}

bool ConfigReader::endElement(const QString& nsuri, const QString& name,
                              const QString& qname)
{
    switch (state) {
    case inActions:
        if (qname == "actions")
            state = inLimbo;
        break;
    case inHighlight:
        if (qname == "highlight")
            state = inActions;
        break;
    case inTrackerCondition: {
        if (qname == "tracker-condition")
            state = inActions;
        Tracker_cfg.insert(condName, bindParams (sparqlSnippet));
        break;
    }
    default:
        break;
    }
    return true;
}

#undef fail

// Constructs Highlighter_cfg from mimeToRegexp and mimeToParent.  Sorts the
// regexps topologically so that the special cases appear before the general
// cases.
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
        // Insert, and also remove from mimeToRegexp to note it has been
        // inserted.
        Highlighter_cfg.prepend(
            qMakePair(QString(HighlighterMimeClass) + toInsert,
                      QRegExp(mimeToRegexp.take(toInsert))));
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
        QXmlSimpleReader reader;
        reader.setContentHandler(&handler);
        reader.setErrorHandler(&handler);
        if (!reader.parse(QXmlInputSource(&file))) {
            LCA_WARNING << "failed to parse" << file.fileName();
            continue;
        }
    }

    // Sort the regexps topologially: each regexp (e.g., a specialized url)
    // before its parent (e.g., a more general url)
    sortRegexps();
    mimeToRegexp.clear();
    mimeToParent.clear();
}

} // end anon namespace

/// Returns the highlighter configuration map of (mimetype, regexp) read from
/// the configuration files.
const QList<QPair<QString, QRegExp> >& ContentAction::Internal::highlighterConfig()
{
    readConfig();
    return Highlighter_cfg;
}

/// Returns the map from friendly names -> sparql snippets.
const QHash<QString, QString>& ContentAction::Internal::trackerConditions()
{
    readConfig();
    return Tracker_cfg;
}

QString ContentAction::Internal::bindParams(const QString &str)
{
    static bool initialized = false;
    static QString manufacturer;
    static QString model;

    if (!initialized) {
        QSystemDeviceInfo sid;
        manufacturer = sid.manufacturer();
        model = sid.model();
        initialized = true;
    }
    
    QString res = str;
    return res.replace("@MANUFACTURER@", manufacturer).replace("@MODEL@", model);
}
