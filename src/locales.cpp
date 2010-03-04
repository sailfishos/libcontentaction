/*
 * Copyright (C) 2010 Nokia Corporation.
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
#include <QCoreApplication>
#include <QTranslator>
#include <QByteArray>
#include <QDebug>

#include <DuiLocale>
#include <DuiGConfItem>

namespace {

using namespace ContentAction::Internal;

class LocaleWatcher: public QObject {
    Q_OBJECT
public:
    LocaleWatcher();

private slots:
    void reloadTranslators();

private:
    DuiLocale *locale;
};

// Installs (or re-installs) QCoreApplication translators based on the
// current DuiLocale.
//
// The default search directory might be overridden with the
// CONTENTACTION_L10N_DIR environment variable.
void LocaleWatcher::reloadTranslators()
{
    static QList<QTranslator *> cur_translators;
    static QString l10ndir;

    if (l10ndir.isEmpty()) {
        QByteArray dir = qgetenv("CONTENTACTION_L10N_DIR");
        if (!dir.isEmpty())
            l10ndir = QString::fromLocal8Bit(dir);
        else
            l10ndir = QString::fromLocal8Bit(DEFAULT_L10N_DIR);
    }

    while (!cur_translators.isEmpty()) {
        QTranslator *tr = cur_translators.takeFirst();
        QCoreApplication::removeTranslator(tr);
        delete tr;
    }
    foreach (const QString& qmfn, translationsConfig()) {
        QTranslator *tr = new QTranslator();
        QString fullfn = qmfn + "_" + locale->name();
        if (!tr->load(fullfn, l10ndir))
            LCA_WARNING << "failed to load translation:" << fullfn;
        cur_translators << tr;
        QCoreApplication::installTranslator(tr);
    }
}

LocaleWatcher::LocaleWatcher()
{
    DuiGConfItem languageItem("/Dui/i18n/Language");
    locale = new DuiLocale(languageItem.value().toString(), this);
    connect(locale, SIGNAL(settingsChanged()),
            this, SLOT(reloadTranslators()));
    locale->connectSettings();
    reloadTranslators();
}

#include "locales.moc"
} // end anon namespace

void ContentAction::Internal::initializeLocales()
{
    static LocaleWatcher *instance = 0;
    if (!instance)
        instance = new LocaleWatcher();
}
