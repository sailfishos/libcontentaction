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
#include <QtCore/QMimeDatabase>

DeclarativeContentAction::DeclarativeContentAction(QObject *parent)
    : QObject(parent)
    , m_error(NoError)
    , m_mimeDatabase(nullptr)
{
}

DeclarativeContentAction::~DeclarativeContentAction()
{
    delete m_mimeDatabase;
}

DeclarativeContentAction::Error DeclarativeContentAction::error() const
{
    return m_error;
}

QString DeclarativeContentAction::mimeType() const
{
    return m_mimeType;
}

void DeclarativeContentAction::updateMimeType(const QUrl &url)
{
    if (!m_mimeDatabase) {
        m_mimeDatabase = new QMimeDatabase;
    }

    QString mimeType = m_mimeDatabase->mimeTypeForUrl(url).name();
    if (m_mimeType != mimeType) {
        m_mimeType = mimeType;
        emit mimeTypeChanged();
    }
}

bool DeclarativeContentAction::trigger(const QUrl &url)
{
    m_error = NoError;
    QString oldMimeType = m_mimeType;
    m_mimeType.clear();

    if (!url.isValid()) {
        qWarning() << Q_FUNC_INFO << "Invalid URL!";
        m_error = InvalidUrl;
        emit errorChanged();
        if (m_mimeType != oldMimeType) {
            emit mimeTypeChanged();
        }
        return false;
    }

    if (url.isLocalFile()) {
        QFile file(url.toLocalFile());
        if (!file.exists()) {
            qWarning() << Q_FUNC_INFO << "File doesn't exist!";
            m_error = FileDoesNotExist;
            emit errorChanged();
            if (m_mimeType != oldMimeType) {
                emit mimeTypeChanged();
            }

            return false;
        }

        if (file.size() == 0) {
            m_error = FileIsEmpty;
            emit errorChanged();
            if (m_mimeType != oldMimeType) {
                emit mimeTypeChanged();
            }
            return false;
        }

        ContentAction::Action action = ContentAction::Action::defaultActionForFile(url);
        if (!action.isValid()) {
            m_error = FileTypeNotSupported;
            emit errorChanged();
            updateMimeType(url);
            return false;
        }

        action.trigger();
    } else {
        ContentAction::Action action = ContentAction::Action::defaultActionForScheme(url.toString());
        if (!action.isValid()) {
            m_error = UrlSchemeNotSupported;
            updateMimeType(url);
            emit errorChanged();
            return false;
        }

        action.trigger();
    }

    updateMimeType(url);
    return true;
}

