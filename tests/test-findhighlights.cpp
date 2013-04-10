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

// This test utilized the regexps in data/hl-examples.xml

#include "contentaction.h"

#include <QObject>
#include <QTest>
#include <QDebug>

using namespace ContentAction;

class TestFindHighlights : public QObject
{
    Q_OBJECT
public Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private Q_SLOTS:
    void allHighlights();
    void noOverlap();

    void nextHighlight();
};

void TestFindHighlights::initTestCase()
{
    // set the environment variables but don't overwrite it if it exists
    setenv("CONTENTACTION_ACTIONS", CONTENTACTION_ACTIONS, 0);
    setenv("XDG_DATA_HOME", XDG_DATA_HOME, 0);
}

void TestFindHighlights::cleanupTestCase()
{
}

void TestFindHighlights::init()
{
}

void TestFindHighlights::cleanup()
{
}

void TestFindHighlights::allHighlights()
{
    {
        QString text = "here is something to match: foobarX and here another: foobazX";

        // There are 3 relevant regexps for this test: foo\w*, foobar\w* and
        // foobaz\w*, the latter 2 are specializations. "foo" doesn't appear as a
        // match since the specializations are preferred, and regexps cannot
        // overlap.
        QStringList expectedMatches;
        expectedMatches << "foobarX" << "foobazX";

        QList<QPair<int, int> > res = Action::findHighlights(text);

        QCOMPARE(res.size(), expectedMatches.size());
        for (int i = 0; i < expectedMatches.size(); ++i) {
            QCOMPARE(res[i].first, text.indexOf(expectedMatches[i]));
            QCOMPARE(res[i].second, expectedMatches[i].length());
        }
    }

    {
        QString text = "nothing relevant in this text";

        QList<QPair<int, int> > res = Action::findHighlights(text);

        QCOMPARE(res.size(), 0);
    }
}

void TestFindHighlights::noOverlap()
{
    QString text = "regexps as endings are okfoobarfine but cannot overlap: foobarfoobazfoo";

    // There are 3 relevant regexps for this test: foo\w*, foobar\w* and
    // foobaz\w*, the latter 2 are specializations.

    QStringList expectedMatches;
    expectedMatches << "foobarfine" << "foobarfoobazfoo";

    QList<QPair<int, int> > res = Action::findHighlights(text);

    QCOMPARE(res.size(), expectedMatches.size());
    for (int i = 0; i < expectedMatches.size(); ++i) {
        QCOMPARE(res[i].first, text.indexOf(expectedMatches[i]));
        QCOMPARE(res[i].second, expectedMatches[i].length());
    }
}

void TestFindHighlights::nextHighlight()
{
    QString text = "I have a cat, I don't have a catepillar, my catalyzator is broken.";

    // There is 1 relevant regexp for this test: cat\w*

    {
        QString expectedMatch = "cat";
        QPair<int, int> res = Action::findNextHighlight(text,
                                                        text.indexOf("have"));
        QCOMPARE(res.first, text.indexOf(expectedMatch));
        QCOMPARE(res.second, expectedMatch.length());
    }
    {
        QString expectedMatch = "catepillar";
        QPair<int, int> res = Action::findNextHighlight(text,
                                                        text.indexOf("don't"));
        QCOMPARE(res.first, text.indexOf(expectedMatch));
        QCOMPARE(res.second, expectedMatch.length());
    }
    {
        QPair<int, int> res = Action::findNextHighlight(text,
                                                        text.indexOf("is"));
        QCOMPARE(res.first, -1);
        QCOMPARE(res.second, -1);
    }

}

QTEST_MAIN(TestFindHighlights)
#include "test-findhighlights.moc"
