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

#include "contentaction.h"

#include <QObject>
#include <QTest>
#include <QDebug>
#include <QThread>

#include <QDir>

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
private:
    QString tempApplications;
};

void TestMimeDefaults::initTestCase()
{
    const QByteArray temp = QDir::home().filePath("mimedefaults-test").toLocal8Bit();

    setenv("XDG_DATA_HOME", temp, 1);
    tempApplications = QString(temp) + "/applications";
    QDir(".").mkpath(tempApplications);
}

void TestMimeDefaults::cleanupTestCase()
{
    // Unfortunately, no rmtree in Qt/C++.
    QFile file(tempApplications + "/defaults.list");
    file.remove();
    QDir(".").rmpath(QString(tempApplications));
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

    {
        ContentAction::setMimeDefault("text/plain", "ubermeego");
        Action a = ContentAction::defaultActionForMime("text/plain");
        QCOMPARE(a.name(), QString("ubermeego"));
    }

    QThread::sleep(1); // time resolution (not only) on ext3 is 1s (see readChangedKeyValueFiles())

    // Do it again for another app, just in case ubermeego was already the
    // default for some reason.
    {
        ContentAction::setMimeDefault("text/plain", "ubermimeopen");
        Action a = ContentAction::defaultActionForMime("text/plain");
        QCOMPARE(a.name(), QString("ubermimeopen"));
    }
}

QTEST_MAIN(TestMimeDefaults)
#include "test-mimedefaults.moc"
