#include <QDebug>
#include <QDir>

#include <contentinfo.h>

#define EXPECT(expr,msg...)					\
  expect((expr), #expr, __FILE__, __LINE__, ##msg, NULL)

void
expect (int b, const char *expr, const char *file, int line,
        const char *fmt, ...)
{
  if (!b)
    {
      printf ("%s:%d: ", file, line);
      if (fmt)
	{
	  va_list ap;
	  va_start (ap, fmt);
	  vprintf (fmt, ap);
	  va_end (ap);
	}
      else
	printf ("Expected %s", expr);
      printf ("\n");
      exit (1);
    }
}

void
dump_info (ContentInfo &info)
{
  if (info.isValid())
    {
      qDebug() << "mime:" << info.mimeType();
      qDebug() << "desc:" << info.description();
      qDebug() << "icon:" << info.icon();
    }
  else
    qDebug() << "invalid";
}

void
test_mime_info ()
{
  ContentInfo info = ContentInfo::forMime ("text/plain");

  EXPECT (info.isValid());
  EXPECT (info.mimeType() == "text/plain");
  EXPECT (info.description() == "plain text document");
  // EXPECT (info.icon() == "icon-m-content-file-unknown");
}

void
test_file_info ()
{
  ContentInfo info = ContentInfo::forFile (QUrl::fromLocalFile(QDir::currentPath() + "/test-image.png"));

  EXPECT (info.isValid());
  EXPECT (info.mimeType() == "image/png");
  EXPECT (info.description() == "PNG image");
  // EXPECT (info.icon() == "icon-m-content-file-unknown");
}

void
test_tracker_info ()
{
  {
    ContentInfo info = ContentInfo::forTracker ("b.image");
    
    EXPECT (info.isValid());
    EXPECT (info.mimeType() == "image/png");
    EXPECT (info.description() == "PNG image");
    // EXPECT (info.icon() == "icon-m-content-file-unknown");
  }

  {
    ContentInfo info = ContentInfo::forTracker ("a.music");
    
    EXPECT (info.isValid());
    EXPECT (info.mimeType() == "audio/mpeg");
    EXPECT (info.description() == "MP3 audio");
    // EXPECT (info.icon() == "icon-m-content-file-unknown");
  }

  {
    ContentInfo info = ContentInfo::forTracker ("urn:test:calendarevent");
    
    EXPECT (!info.isValid());
  }
}

void
test_bytes_info ()
{
  QFile file("./test-image.png");
  file.open (QIODevice::ReadOnly);
  QByteArray content = file.read (200);

  ContentInfo info = ContentInfo::forBytes (content);

  EXPECT (info.isValid());
  EXPECT (info.mimeType() == "image/png");
  EXPECT (info.description() == "PNG image");
  // EXPECT (info.icon() == "icon-m-content-file-unknown");
}

void
test_iodevice_info ()
{
  QFile file("./test-image.png");
  file.open (QIODevice::ReadOnly);

  ContentInfo info = ContentInfo::forIODevice (file);

  EXPECT (info.isValid());
  EXPECT (info.mimeType() == "image/png");
  EXPECT (info.description() == "PNG image");
  // EXPECT (info.icon() == "icon-m-content-file-unknown");
}

int
main (int argc, char **argv)
{
  test_mime_info ();
  test_file_info ();
  test_tracker_info ();
  test_bytes_info ();
  test_iodevice_info ();

  exit (0);
}
