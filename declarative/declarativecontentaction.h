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

#ifndef DECLARATIVECONTENTACTION_H
#define DECLARATIVECONTENTACTION_H

#include <QtCore/QObject>

QT_BEGIN_NAMESPACE
class QMimeDatabase;
QT_END_NAMESPACE


class DeclarativeContentAction: public QObject
{
    Q_OBJECT

    Q_PROPERTY(Error error READ error NOTIFY errorChanged)
    Q_PROPERTY(QString mimeType READ mimeType NOTIFY mimeTypeChanged)

    Q_ENUMS(Error)

public:
    enum Error {
        NoError,
        FileTypeNotSupported,
        FileDoesNotExist,
        FileIsEmpty,
        UrlSchemeNotSupported,
        InvalidUrl
    };

    explicit DeclarativeContentAction(QObject *parent = 0);
    virtual ~DeclarativeContentAction();

    Q_INVOKABLE bool trigger(const QUrl &url);

    Error error() const;
    QString mimeType() const;

signals:
    void errorChanged();
    void mimeTypeChanged();

private:
    void updateMimeType(const QUrl &url);

    Error m_error;
    QString m_mimeType;
    QMimeDatabase *m_mimeDatabase;
};

#endif // DECLARATIVECONTENTACTION_H
