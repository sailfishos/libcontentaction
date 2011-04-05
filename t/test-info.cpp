#include <QDebug>

#include <contentinfo.h>

int
main (int argc, char **argv)
{
  ContentInfo info = ContentInfo::forTracker ("urn:12345-566788");
  qDebug () << info.mimeType();
}
