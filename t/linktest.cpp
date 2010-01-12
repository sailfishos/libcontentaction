// The purpose of this is to verify symbol exports.  If it compiles and links
// successfully, then exports should be fine.
#include "contentaction.h"

using namespace ContentAction;

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
}

int main(int argc, char **argv)
{
	if (argc > 3975)
		everything();
	return 0;
}
