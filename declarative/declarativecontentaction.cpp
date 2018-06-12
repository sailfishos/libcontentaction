/*
 * Copyright (C) 2015 Jolla Ltd.
 *
 * Contact: Aaron McCarthy <aaron.mccarthy@jollamobile.com>
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

#include "declarativecontentaction.h"

#include "contentaction.h"

#include <QtCore/QUrl>
#include <QtCore/QFile>
#include <QtCore/QDebug>

DeclarativeContentAction::DeclarativeContentAction(QObject *parent)
    : QObject(parent)
    , m_error(NoError)
{
}

DeclarativeContentAction::Error DeclarativeContentAction::error() const
{
    return m_error;
}

bool DeclarativeContentAction::trigger(const QUrl &url)
{
    m_error = NoError;

    if (!url.isValid()) {
        qWarning() << Q_FUNC_INFO << "Invalid URL!";
        m_error = InvalidUrl;
        emit errorChanged();
        return false;
    }

    if (url.isLocalFile()) {
        QString localFile = url.toLocalFile();
        if (!QFile::exists(localFile)) {
            qWarning() << Q_FUNC_INFO << "File doesn't exist!";
            m_error = FileDoesNotExist;
            emit errorChanged();
            return false;
        }

        QFile file(localFile);
        if (file.open(QIODevice::ReadOnly) && file.size() == 0) {
            m_error = FileIsEmpty;
            emit errorChanged();
            file.close();
            return false;
        }

        if (file.openMode() != QIODevice::NotOpen) {
            file.close();
        }

        ContentAction::Action action = ContentAction::Action::defaultActionForFile(url);
        if (!action.isValid()) {
            m_error = FileTypeNotSupported;
            emit errorChanged();
            return false;
        }

        action.trigger();
    } else {
        ContentAction::Action action = ContentAction::Action::defaultActionForScheme(url.toString());
        if (!action.isValid()) {
            m_error = UrlSchemeNotSupported;
            emit errorChanged();
            return false;
        }

        action.trigger();
    }

    return true;
}

