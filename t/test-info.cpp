#include <QDebug>

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
test_mime_info ()
{
  ContentInfo info = ContentInfo::forMime ("text/plain");

  EXPECT (info.isValid());
  EXPECT (info.mimeType() == "text/plain");
  EXPECT (info.description() == "A text/plain file");
  EXPECT (info.icon() == "icon-m-content-file-unknown");
}

void
test_tracker_info ()
{
  {
    ContentInfo info = ContentInfo::forTracker ("b.image");
    
    EXPECT (info.isValid());
    EXPECT (info.mimeType() == "image/png");
    EXPECT (info.description() == "A image/png file");
    EXPECT (info.icon() == "icon-m-content-file-unknown");
  }

  {
    ContentInfo info = ContentInfo::forTracker ("a.music");
    
    EXPECT (info.isValid());
    EXPECT (info.mimeType() == "audio/mpeg");
    EXPECT (info.description() == "A audio/mpeg file");
    EXPECT (info.icon() == "icon-m-content-file-unknown");
  }

  {
    ContentInfo info = ContentInfo::forTracker ("urn:test:calendarevent");
    
    EXPECT (!info.isValid());
  }
}

int
main (int argc, char **argv)
{
  test_mime_info ();
  test_tracker_info ();

  exit (0);
}
