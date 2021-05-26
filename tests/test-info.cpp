#include <QDebug>
#include <QDir>
#include <QtTest/QtTest>

#include <contentinfo.h>

void
dump_info (ContentInfo &info)
{
  if (info.isValid())
    {
      qDebug() << "mime:" << info.mimeType();
      qDebug() << "desc:" << info.typeDescription();
      qDebug() << "icon:" << info.typeIcon();
    }
  else
    qDebug() << "invalid";
}

class TestInfo : public QObject
{
  Q_OBJECT

private Q_SLOTS:
  
  void
  test_mime_info ()
  {
    ContentInfo info = ContentInfo::forMime ("text/plain");
    
    QVERIFY (info.isValid());
    QVERIFY (info.mimeType() == "text/plain");
    QVERIFY (info.typeDescription() == "plain text document");
    // QVERIFY (info.typeIcon() == "icon-m-content-file-unknown");
  }

  void
  test_file_info ()
  {
    ContentInfo info = ContentInfo::forFile (QUrl::fromLocalFile(QDir::currentPath() + "/test-image.png"));
    
    QVERIFY (info.isValid());
    QVERIFY (info.mimeType() == "image/png");
    QVERIFY (info.typeDescription() == "PNG image");
    // QVERIFY (info.typeIcon() == "icon-m-content-file-unknown");
  }

  void
  test_bytes_info ()
  {
    QFile file("./test-image.png");
    file.open (QIODevice::ReadOnly);
    QByteArray content = file.read (200);
    
    ContentInfo info = ContentInfo::forData (content);
    
    QVERIFY (info.isValid());
    QVERIFY (info.mimeType() == "image/png");
    QVERIFY (info.typeDescription() == "PNG image");
    // QVERIFY (info.typeIcon() == "icon-m-content-file-unknown");
  }
};

QTEST_MAIN(TestInfo);
#include "test-info.moc"
