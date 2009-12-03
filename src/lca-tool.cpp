#include "contentaction.h"

#include <unistd.h>
#include <QTextStream>

void usage(char* prog)
{
    QTextStream out(stdout);
    out << "Usage: " << prog << "[--print|-p] URI.." << endl
        << "       " << prog << "--invoke|-i interface.method URI.." << endl
        << "       " << prog << "--invokedefault|-d URI.." << endl;
}

int main(int argc, char** argv)
{
    if (argc == 1) {
        usage(argv[0]);
        return 1;
    }
    QStringList args;
    for (int i = 1; i < argc; ++i) {
        args << argv[i];
    }

    int todo = 0;
    QString actionName = "";
    if (args.size() > 0 && (args[0] == "--invoke" || args[0] == "-i")) {
        if (argc < 3) {
            usage(argv[0]);
            return 1;
        }
        args.takeFirst();
        actionName = args.takeFirst();
        todo = 1;
    }

    if (args.size() > 0 && (args[0] == "--invokedefault" || args[0] == "-d")) {
        args.takeFirst();
        todo = 2;
    }

    if (args.size() > 0 && (args[0] == "--print" || args[0] == "-p"))
        args.takeFirst();

    QTextStream out(stdout);
    if (todo == 0 || todo == 1) {
        QList<ContentAction> actions = ContentAction::actions(args);
        foreach (const ContentAction& action, actions) {
            if (todo == 0) // Print
                out << action.name() << endl;
            else if (todo == 1 && actionName == action.name()) // Invoke
                action.trigger();
        }
    }
    else {
        // Default action
        ContentAction defAction = ContentAction::defaultAction(args);
        defAction.trigger();
    }

    return 0;
}
