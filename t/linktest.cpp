// The purpose of this is to verify symbol exports.  If it compiles and links
// successfully, then exports should be fine.
#include "contentaction.h"
#include "internal.h"

#include <MDesktopEntry>

using namespace ContentAction;
using namespace ContentAction::Internal;

void everything()
{
    Action a = Action::defaultAction("satisf action");
    a.trigger();
    a.isValid();
    a.name();
    a.icon();
    Action b = Action::defaultAction(QStringList() << "and" << "some");
    Action c(b);
    c = a;
    QList<Action> piff = Action::actions(QStringList() << "and" << "some");
    QList<Action> paff = Action::actions("concatenation");

    Action::highlight("foo bar baz");

    Action::actionsForFile(QUrl("file:///somewhere/over/the/rainbow"));
    Action::defaultActionForFile(QUrl("file:///somewhere/over/the/rainbow"));

    QString x = a.localizedName();

    setMimeDefault("foo", "bar");
    QList<Action> list = actionsForMime("foo");
    setMimeDefault("foo", list[0]);

    QSharedPointer<MDesktopEntry> m(new MDesktopEntry("some.desktop"));
    Action d = Action::launcherAction(m, QStringList() << "param1" << "param2");

    Action e = Action::launcherAction("some.desktop", QStringList() << "param1" << "param2");
}

int main(int argc, char **argv)
{
    if (argc > 3975)
        everything();
    return 0;
}
