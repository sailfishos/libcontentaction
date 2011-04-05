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

#include "contentinfo.h"
#include "internal.h"

/*!
  \class ContentInfo

  \brief ContentInfo provides information about a content object, such
  as an icon and a description.
*/

struct ContentInfo::Private {
  bool isValid;
  QString mimeType;
  QString icon;
  QString description;
};

ContentInfo::ContentInfo ()
  : priv(new Private)
{
  priv->isValid = false;
}

ContentInfo::ContentInfo(Private *priv)
  : priv(priv)
{
}

ContentInfo::ContentInfo(const ContentInfo& other)
  : priv(other.priv)
{
}

ContentInfo& ContentInfo::operator=(const ContentInfo& other)
{
  priv = other.priv;
  return *this;
}

ContentInfo::~ContentInfo ()
{
}

bool
ContentInfo::isValid () const
{
  return priv->isValid;
}

QString
ContentInfo::mimeType () const
{
  return priv->mimeType;
}

QString
ContentInfo::description () const
{
  return priv->description;
}

QString
ContentInfo::icon () const
{
  return priv->icon;
}

ContentInfo
ContentInfo::forMime (const QString &mimeType)
{
  Private *priv = new Private;
  priv->isValid = true;
  priv->mimeType = mimeType;
  priv->description = "A " + mimeType + " file";
  priv->icon = "icon-m-content-file-unknown";
  return ContentInfo(priv);
}

ContentInfo
ContentInfo::forTracker (const QString &tracker_uri)
{
  QStringList urlAndMime;
  if (ContentAction::Internal::mimeAndUriFromTracker(QStringList() << tracker_uri, urlAndMime))
    return forMime (urlAndMime[1]);
  else
    return ContentInfo ();
}
