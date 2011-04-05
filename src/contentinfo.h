/*
 * Copyright (C) 2011 Nokia Corporation.
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

#ifndef CONTENTINFO_H
#define CONTENTINFO_H

#include <QString>
#include <QUrl>
#include <QSharedPointer>
#include <QIODevice>
#include <QByteArray>

#ifndef LCA_EXPORT
# if defined(LCA_BUILD)
#  define LCA_EXPORT Q_DECL_EXPORT
# else
#  define LCA_EXPORT Q_DECL_IMPORT
# endif
#endif

class LCA_EXPORT ContentInfo
{
public:
    bool isValid() const;
    QString mimeType() const;
    QString description() const;
    QString icon() const;

    ContentInfo();
    ~ContentInfo();
    ContentInfo(const ContentInfo& other);
    ContentInfo& operator=(const ContentInfo& other);

    static ContentInfo forMime (const QString &mimeType);
    static ContentInfo forTracker (const QString &trackerUri);
    static ContentInfo forFile (const QUrl &file);
    static ContentInfo forBytes (const QIODevice &io);
    static ContentInfo forBytes (const QByteArray &arr);
    
private:
    struct Private;
    QSharedPointer<Private> priv;
    ContentInfo(Private *priv);
};

#endif
