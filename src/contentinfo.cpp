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

#include <gio/gio.h>

/*!
  \class ContentInfo

  \brief ContentInfo provides information about a content object,
  including its mime type, and a generic description and icon.
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

/// Determines whether this ContentInfo is valid or not.  Invalid
/// ContentInfo objects are returned when no information about a
/// object could be found.
///
/// Invalid ContentInfo instances can still be accessed, but they will
/// return empty strings for mimeType, typeDescription, and typeIcon.
bool
ContentInfo::isValid () const
{
  return priv->isValid;
}

/// Returns the mime type of this content object
QString
ContentInfo::mimeType () const
{
  return priv->mimeType;
}

/// Returns a one-line, localized description of the type of the
/// content object.
QString
ContentInfo::typeDescription () const
{
  return priv->description;
}

/// Returns the name of an icon to represent the type of this content
/// object.
QString
ContentInfo::typeIcon () const
{
  return priv->icon;
}

/// Returns information for the given mime type \a mimeType.
ContentInfo
ContentInfo::forMime (const QString &mimeType)
{
  g_type_init ();

  char *contentType = g_content_type_from_mime_type (mimeType.toUtf8());

  Private *priv = new Private;
  priv->isValid = true;
  priv->mimeType = mimeType;
  if (contentType)
    {
      GIcon *icon = g_content_type_get_icon (contentType);
      if (G_IS_THEMED_ICON(icon))
        {
          const gchar *const *names = g_themed_icon_get_names (G_THEMED_ICON(icon));
          priv->icon = names[0];
        }
      priv->description = g_content_type_get_description (contentType);
      g_object_unref (icon);
      g_free (contentType);
    }
  return ContentInfo(priv);
}

/// Returns information for the file identified by \a url.  The file
/// does not need to exist.  If it does, its content will be used to
/// guess its type; otherwise only the filename will be used.
ContentInfo
ContentInfo::forFile (const QUrl &url)
{
  QString mime = ContentAction::Internal::mimeForFile (url);
  if (!mime.isEmpty())
    return forMime (mime);
  else
    return ContentInfo();
}

/// Returns information for the Tracker object identified by \a
/// tracker_uri.
ContentInfo
ContentInfo::forTracker (const QString &tracker_uri)
{
  QStringList urlAndMime;
  if (ContentAction::Internal::mimeAndUriFromTracker(QStringList() << tracker_uri, urlAndMime))
    return forMime (urlAndMime[1]);
  else
    return ContentInfo();
}

/// Returns information for the given \a bytes.  The \a bytes are
/// assumed to be the first few bytes of a content object, and its
/// type is guessed from them.
ContentInfo
ContentInfo::forData (const QByteArray &bytes)
{
   g_type_init ();
  
   char *content_type = g_content_type_guess (NULL, (const guchar *)bytes.constData(), bytes.size(), NULL);
   if (content_type)
     {
       ContentInfo info = forMime (g_content_type_get_mime_type (content_type));
       free (content_type);
       return info;
     }
   else
     return ContentInfo();
}
