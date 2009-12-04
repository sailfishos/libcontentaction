/*
 * Copyright (C) 2009 Nokia Corporation.
 *
 * Contact: Marius Vollmer <marius.vollmer@nokia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */
#include "contentaction.h"
#include "internal.h"

#include <unistd.h>
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
        "  -c|--classes                  print the classes of the URIs\n"
        "Return values:\n"
        "  0   success\n"
        "  1   no arguments given\n"
        "  2   problem with command arguments\n"
        "  3   tried to invoke an action not applicable for the given URIs\n"
        "  4   no default action exists for the given URIs\n";
}

int main(int argc, char **argv)
{
    QTextStream err(stderr), out(stdout);

    if (argc == 1) {
        usage(argv[0]);
        return 1;
    }

    QStringList args;
    for (int i = 1; i < argc; ++i)
        args << QString(argv[i]);

    Action todo = PrintHelp;
    QString actionName;
    while (!args.isEmpty()) {
        QString arg = args.takeFirst();
        if (!arg.startsWith("-"))             // end of options
            break;

        if (arg == "-h" || arg == "--help") {
            usage(argv[0]);
            return 1;
        }
        if (arg == "-p" || arg == "--print") {
            todo = PrintActions;
            break;
        }
        if (arg == "-i" || arg == "--invoke") {
            todo = Invoke;
            if (args.isEmpty()) {
                err << "an action must be given when using " << arg << endl;
                return 2;
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
        return 2;
    }

    switch (todo) {
    case PrintHelp:
        usage(argv[0]);
        return 1;
        break;
    case PrintActions:
    case Invoke: {
        QList<ContentAction> actions = ContentAction::actions(args);
        foreach (const ContentAction& action, actions) {
            if (todo == PrintActions) {
                out << action.name() << endl;
            } else if (todo == Invoke && actionName == action.name()) {
                action.trigger();
                return 0;
            }
        }
        if (todo == Invoke) {
            err << "action '" << actionName << "'is not applicable\n";
            return 3;
        }
        break;
    }
    case InvokeDefault: {
        ContentAction defAction = ContentAction::defaultAction(args);
        if (!defAction.isValid()) {
            err << "no default action for the given URIs\n";
            return 4;
        }
        defAction.trigger();
    }
    case PrintClasses: {
        foreach (const QString& cls, classesOf(args[0]))
            out << cls << endl;
    }
    }
    return 0;
}
