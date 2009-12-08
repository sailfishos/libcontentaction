// The purpose of this is to verify symbol exports.  If it compiles and links
// successfully, then exports should be fine.
#include "contentaction.h"

using namespace ContentAction;

int main()
{
    return 0;
    Action a = Action::defaultAction("satisf action");
    a.trigger();
    a.setAsDefault();
    a.isDefault();
    a.canBeDefault();
    a.isValid();
    a.name();
    Action b = Action::defaultAction(QStringList() << "and" << "some");
    QList<Action> piff = Action::actions(QStringList() << "and" << "some");
    QList<Action> paff = Action::actions("concatenation");
}
