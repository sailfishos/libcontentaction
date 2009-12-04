#include "contentaction.h"
#include "internal.h"

#include <unistd.h>
#include <QCoreApplication>
#include <QTextStream>

enum Action {
    PrintHelp,
    PrintActions,
    Invoke,
    InvokeDefault,
    PrintClasses,
};

void usage(char *prog)
{
    QTextStream err(stderr);
    err << "Usage: " << prog << " [OPTIONS] URI [URIS...]\n"
        "  -h|--help                     print this text\n"
        "  -p|--print                    print the applicable actions\n"
        "  -i|--invoke=INTERFACE.METHOD  invoke the specified action\n"
        "  -d|--invokedefault            invoke the default action\n"
        "  -c|--classes                  print the classes of the URIs\n";
    exit(1);
}

int main(int argc, char **argv)
{
    QTextStream err(stderr), out(stdout);
    if (argc == 1)
        usage(argv[0]);

    QStringList args;
    for (int i = 1; i < argc; ++i)
        args << QString(argv[i]);

    Action todo = PrintHelp;
    QString actionName;
    while (!args.isEmpty()) {
        QString arg = args.takeFirst();
        if (!arg.startsWith("-"))             // end of options
            break;

        if (arg == "-h" || arg == "--help")
            usage(argv[0]);
        if (arg == "-p" || arg == "--print") {
            todo = PrintActions;
            break;
        }
        if (arg == "-i" || arg == "--invoke") {
            todo = Invoke;
            if (args.isEmpty()) {
                err << "an action must be given when using " << arg << endl;
                exit(2);
            }
            actionName = args.takeFirst();
            break;
        }
        if (arg == "-d" || arg == "--invokedefault") {
            todo = InvokeDefault;
            break;
        }
        if (arg == "-c" || arg == "--classes") {
            todo = PrintClasses;
            break;
        }
    }

    if (args.isEmpty()) {
        err << "no URIs given\n";
        exit(2);
    }

    switch (todo) {
    case PrintHelp:
        usage(argv[0]);
        break;
    case PrintActions:
    case Invoke: {
        QList<ContentAction> actions = ContentAction::actions(args);
        foreach (const ContentAction& action, actions) {
            if (todo == PrintActions)
                out << action.name() << endl;
            else if (todo == Invoke && actionName == action.name())
                action.trigger();
        }
        break;
    }
    case InvokeDefault: {
        ContentAction defAction = ContentAction::defaultAction(args);
        defAction.trigger();
    }
    case PrintClasses: {
        foreach (const QString& cls, classesOf(args[0]))
            out << cls << endl;
    }
    }
    return 0;
}
