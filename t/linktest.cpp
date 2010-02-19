// The purpose of this is to verify symbol exports.  If it compiles and links
// successfully, then exports should be fine.
#include "contentaction.h"
#include "internal.h"

using namespace ContentAction;
using namespace ContentAction::Internal;

void everything()
{
    Action a = Action::defaultAction("satisf action");
    a.trigger();
    a.setAsDefault();
    a.isDefault();
    a.canBeDefault();
    a.isValid();
    a.name();
    Action b = Action::defaultAction(QStringList() << "and" << "some");
    Action c(b);
    c = a;
    QList<Action> piff = Action::actions(QStringList() << "and" << "some");
    QList<Action> paff = Action::actions("concatenation");

    Action::highlight("foo bar baz");
    const HighlighterMap& m = highlighterConfig();
    Q_UNUSED(m);
    a = highlightAction("foo", "bar");

    Action::actionsForFile(QUrl("file:///somewhere/over/the/rainbow"));
    Action::defaultActionForFile(QUrl("file:///somewhere/over/the/rainbow"));

    Action::installTranslators("en_US");
    QString x = a.localizedName();
}

int main(int argc, char **argv)
{
	if (argc > 3975)
		everything();
	return 0;
}
