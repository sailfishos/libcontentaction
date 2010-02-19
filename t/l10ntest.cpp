#include <stdio.h>
#include <contentaction.h>
#include <QCoreApplication>
#include <QList>
#include <QTextStream>

using ContentAction::Action;

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QTextStream out(stdout);
    Action::installTranslators(qgetenv("LANG"));
    QList<Action> alist = Action::actions(QString::fromLocal8Bit(argv[1]));
    foreach (const Action& a, alist)
        out << a.localizedName() << "\n";
    return 0;
}
