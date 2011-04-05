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
test_tracker_info ()
{
  ContentInfo info = ContentInfo::forTracker ("b.image");

  EXPECT (info.isValid());
  EXPECT (info.mimeType() == "image/png");
  EXPECT (info.description() == "Document");
  EXPECT (info.icon() == "icon-content-m-document");
}

int
main (int argc, char **argv)
{
  test_tracker_info ();

  exit (0);
}
