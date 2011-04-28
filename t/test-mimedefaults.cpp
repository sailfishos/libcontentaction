/*
 * Copyright (C) 2010-2011 Nokia Corporation.
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

#include "env.h"

#include "contentaction.h"

#include <QObject>
#include <QTest>
#include <QDebug>

using namespace ContentAction;

class TestMimeDefaults : public QObject
{
    Q_OBJECT
public Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private Q_SLOTS:
    void setMimeDefault();
};

void TestMimeDefaults::initTestCase()
{
    // Set the environment variables but don't overwrite them if they're already
    // set.
    setenv("CONTENTACTION_ACTIONS", CONTENTACTION_ACTIONS, 0);
    setenv("XDG_DATA_HOME", XDG_DATA_HOME, 0);
}

void TestMimeDefaults::cleanupTestCase()
{
}

void TestMimeDefaults::init()
{
}

void TestMimeDefaults::cleanup()
{
    ContentAction::resetMimeDefault("text/plain");
}

void TestMimeDefaults::setMimeDefault()
{
    // The same as test-defaults.py, except that we don't use lca-tool but call
    // the libcontentaction c++ api from here. This way we simulate querying the
    // mime default twice during the run of a process.

    // The waits are needed since the resolution of the "last modified time" is
    // one second.  This will be a problem if somebody sets the mime default
    // twice during the same second, and somebody reads it between those two
    // changes.

    {
        ContentAction::setMimeDefault("text/plain", "ubermeego");
        QTest::qWait(1000);
        Action a = ContentAction::defaultActionForMime("text/plain");
        QCOMPARE(a.name(), QString("ubermeego"));
    }

    // Do it again for another app, just in case ubermeego was already the
    // default for some reason.
    {
        ContentAction::setMimeDefault("text/plain", "ubermimeopen");
        QTest::qWait(1000);
        Action a = ContentAction::defaultActionForMime("text/plain");
        QCOMPARE(a.name(), QString("ubermimeopen"));
    }
}

QTEST_MAIN(TestMimeDefaults)
#include "test-mimedefaults.moc"
