#include <QObject>
#include <QtTest/QtTest>

#include "contentaction.h"

using namespace ContentAction;

void
dump_action (const Action &action)
{
  if (action.isValid())
    {
      qDebug() << "name: " << action.name();
      qDebug() << "local:" << action.localizedName();
      qDebug() << "icon: " << action.icon();
    }
  else
    qDebug() << "invalid";
}

class TestAction : public QObject
{
  Q_OBJECT

private Q_SLOTS:
  void initTestCase()
  {
  }

  void
  test_file_mime_guess ()
  {
    // Test that a mime type of "application/octet-stream" is taken to
    // mean "unknown" and that a better one is guessed.

    QUrl url = QUrl::fromLocalFile(QDir::currentPath() + "/test-image.png");
    Action action1 = Action::defaultActionForFile (url);
    Action action2 = Action::defaultActionForFile (url, "application/octet-stream");
    
    QVERIFY (action1.isValid());
    QVERIFY (action2.isValid());
    QCOMPARE (action1.name(), action2.name());
  }
};


QTEST_MAIN(TestAction);
#include "test-action.moc"
