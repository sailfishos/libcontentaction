#include <QDebug>
#include <QDir>
#include <QtTest/QtTest>

#include <contentinfo.h>

class TestInfo : public QObject
{
  Q_OBJECT

private Q_SLOTS:
    void test_mime_info()
    {
        ContentInfo info = ContentInfo::forMime ("text/plain");

        QVERIFY(info.isValid());
        QCOMPARE(info.mimeType(), QString::fromLatin1("text/plain"));
        QCOMPARE(info.typeDescription(), QString::fromLatin1("Plain text document"));
    }

    void test_file_info() {
        ContentInfo info = ContentInfo::forFile (QUrl::fromLocalFile(QDir::currentPath() + "/test-image.png"));

        QVERIFY(info.isValid());
        QCOMPARE(info.mimeType(), QString::fromLatin1("image/png"));
        QCOMPARE(info.typeDescription(), QString::fromLatin1("PNG image"));
    }

    void test_bytes_info() {
        QFile file("./test-image.png");
        file.open (QIODevice::ReadOnly);
        QByteArray content = file.read (200);

        ContentInfo info = ContentInfo::forData (content);

        QVERIFY(info.isValid());
        QCOMPARE(info.mimeType(), QString::fromLatin1("image/png"));
        QCOMPARE(info.typeDescription(), QString::fromLatin1("PNG image"));
    }
};

QTEST_MAIN(TestInfo);
#include "test-info.moc"
