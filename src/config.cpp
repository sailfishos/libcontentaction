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

namespace ContentAction {

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
    ConfigReader(Associations &_assoc) :
        state(inLimbo), assoc(_assoc)
        {}
    bool startElement(const QString& ns, const QString& name,
                      const QString& qname, const QXmlAttributes &atts);
    bool endElement(const QString& nsuri, const QString& name,
                    const QString& qname);
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
        inLimbo, inActions, inClass,
    } state;

    QString error;
    Associations &assoc;
    QString currentClass;
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
        if (qname != "class")
            fail("expected tag: class");
        state = inClass;
        currentClass = atts.value("name").trimmed();
        if (currentClass.isEmpty())
            fail("expected nonempty class name");
        break;
    case inClass:
        if (qname != "action")
            fail("expected tag: action");
        QString action = atts.value("name").trimmed();
        if (action.isEmpty())
            fail("expected nonempty action name");
        bool isOk = false;
        int weight = atts.value("weight").trimmed().toInt(&isOk);
        if (!isOk)
            fail("expected integer weight");
        if (!assoc.contains(currentClass))
            assoc[currentClass] = QList<QPair<int, QString> >();
        assoc[currentClass].append(QPair<int, QString>(weight, action));
        break;
    }
    return true;
}

bool ConfigReader::endElement(const QString& nsuri, const QString& name,
                                 const QString& qname)
{
    switch (state) {
    case inClass:
        if (qname == "class")
            state = inActions;
        break;
    case inActions:
        if (qname == "actions")
            state = inLimbo;
    default:
        break;
    }
    return true;
}

#undef fail

/// Reads the configuration files for "Nepomuk class - action - weight"
/// association.
const Associations& actionsForClasses()
{
    static Associations assoc;
    static bool read = false;

    if (read)
        return assoc;
    read = true;

    QDir dir(actionPath());
    if (!dir.isReadable()) {
        LCA_WARNING << "cannot read actions from" << dir.path();
        return assoc;
    }
    dir.setNameFilters(QStringList("*.xml"));
    QStringList confFiles = dir.entryList(QDir::Files);
    foreach (const QString& confFile, confFiles) {
        QFile file(dir.filePath(confFile));

        ConfigReader handler(assoc);
        QXmlSimpleReader reader;
        reader.setContentHandler(&handler);
        reader.setErrorHandler(&handler);
        if (!reader.parse(QXmlInputSource(&file))) {
            LCA_WARNING << "failed to parse" << file.fileName();
            continue;
        }
        foreach (const QString& klass, assoc.keys())
            qSort(assoc[klass]);
    }
    return assoc;
}

} // end namespace
