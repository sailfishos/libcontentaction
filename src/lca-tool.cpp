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

#include <stdlib.h>
#include <unistd.h>
#include <QTextStream>
#include <QCoreApplication>
#include <QDebug>
#include <DuiLocale>

using namespace ContentAction;
using namespace ContentAction::Internal;

enum ActionToDo {
    PrintHelp,
    PrintActions,
    Invoke,
    InvokeDefault,
    PrintClasses,
    PrintDefault,
    SetDefault,
    PrintClassDefault,
    SetClassDefault,
    PrintForFile,
    InvokeForFile,
    LaunchFile,
};

void usage(char *prog)
{
    QTextStream err(stderr);
    err << "Usage: " << prog << " [OPTIONS] [URIS...]\n"
        "  -h|--help                          print this text\n"
        "  -p|--print                         print the applicable actions\n"
        "  -i|--invoke ACTION                 invoke the specified action\n"
        "  -I|--invokedefault                 invoke the default action\n"
        "  -c|--classes                       print the classes of the URIs\n"
        "  -d|--default                       print the default action\n"
        "  -s|--setdefault ACTION             set the default action for the given URIs\n"
        "  -D|--classdefault CLASS            print the default action for a Nepomuk class\n"
        "  -S|--setclassdefault ACTION CLASS  set a default action for a Nepomuk class\n"
        "  -f|--printforfile                  print the applicable actions for a file\n"
        "  -F|--invokeforfile FILEACTION      invoke the given action for FILE\n"
        "  -L|--launchfile                    invoke the default action for FILE\n"
        "\n"
        "  --l10n                             use localized names\n"
        "where\n"
        "  ACTION is: INTERFACE.METHOD of the maemo service framework\n"
        "  FILEACTION is: the name of the application (from the .desktop file)\n"
        "Return values:\n"
        "  0   success\n"
        "  1   no arguments given\n"
        "  2   problem with command arguments\n"
        "  3   tried to invoke an action not applicable for the given URIs\n"
        "  4   no default action exists for the given URIs\n"
        "  5   no default action exists for the given Nepomuk class\n"
        "  6   setting a default action for the given Nepomuk class failed\n"
        "  7   an action cannot be set as default action for the given URIs\n";
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QTextStream err(stderr), out(stdout);

    if (argc == 1) {
        usage(argv[0]);
        return 1;
    }

    char *l10npaths = getenv("CONTENTACTION_L10N_PATH");
    if (l10npaths) {
        foreach (const QString& p, QString::fromLocal8Bit(l10npaths).split(':')) {
            qDebug() << "adding path:" << p;
            DuiLocale::addTranslationPath(p);
        }
    } else {
        DuiLocale::addTranslationPath("/usr/share/l10n/dui");
    }

    QStringList args;
    for (int i = 1; i < argc; ++i)
        args << QString(argv[i]);

    ActionToDo todo = PrintHelp;
    bool use_l10n = false;
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
        if (arg == "-I" || arg == "--invokedefault") {
            todo = InvokeDefault;
            break;
        }
        if (arg == "-c" || arg == "--classes") {
            todo = PrintClasses;
            break;
        }
        if (arg == "-D" || arg == "--classdefault") {
            todo = PrintClassDefault;
            break;
        }
        if (arg == "-S" || arg == "--setclassdefault") {
            todo = SetClassDefault;
            if (args.isEmpty()) {
                err << "an action must be given when using " << arg << endl;
                return 2;
            }
            actionName = args.takeFirst();
            break;
        }
        if (arg == "-d" || arg == "--default") {
            todo = PrintDefault;
            break;
        }
        if (arg == "-s" || arg == "--setdefault") {
            todo = SetDefault;
            if (args.isEmpty()) {
                err << "an action must be given when using " << arg << endl;
                return 2;
            }
            actionName = args.takeFirst();
            break;
        }
        if (arg == "-f" || arg == "--printforfile") {
            todo = PrintForFile;
            break;
        }
        if (arg == "-F" || arg == "--invokeforfile") {
            todo = InvokeForFile;
            if (args.isEmpty()) {
                err << "an action must be given when using " << arg << endl;
                return 2;
            }
            actionName = args.takeFirst();
            break;
        }
        if (arg == "-L" || arg == "--launchfile") {
            todo = LaunchFile;
            break;
        }
        if (arg == "--l10n") {
            use_l10n = true;
            continue;
        }
        err << "Unknown option " << arg << endl;
        return 2;
    }

    if (args.isEmpty()) {
        err << "option needs more arguments\n";
        return 2;
    }

    switch (todo) {
    case PrintHelp:
        usage(argv[0]);
        return 1;
        break;
    case PrintActions:
    case SetDefault:
    case Invoke: {
        QList<Action> actions = Action::actions(args);
        foreach (Action action, actions) {
            if (todo == PrintActions) {
                if (use_l10n)
                    out << action.localizedName() << endl;
                else
                    out << action.name() << endl;
            } else if (todo == Invoke && actionName == action.name()) {
                action.trigger();
                return 0;
            }
            else if (todo == SetDefault && actionName == action.name()) {
                action.setAsDefault();
                return 0;
            }
        }
        if (todo == Invoke) {
            err << "action '" << actionName << "'is not applicable\n";
            return 3;
        }
        else if (todo == SetDefault) {
            err << "action '" << actionName << "'cannot be set as default\n";
            return 7;
        }
        break;
    }
    case PrintDefault:
    case InvokeDefault: {
        Action defAction = Action::defaultAction(args);
        if (!defAction.isValid()) {
            err << "no default action for the given URIs\n";
            return 4;
        }
        if (todo == InvokeDefault)
            defAction.trigger();
        else if (todo == PrintDefault) {
            if (use_l10n)
                out << defAction.localizedName() << endl;
            else
                out << defAction.name() << endl;
        }
        break;
    }
    case PrintClasses: {
        foreach (const QString& cls, classesOf(args[0]))
            out << cls << endl;
        break;
    }
    case PrintClassDefault: {
        QString defAction = defaultActionFromGConf(args[0]);
        if (defAction != "")
            out << defAction << endl;
        else {
            err << "no default action for: " << args[0] << endl;
            return 5;
        }
        break;
    }
    case SetClassDefault: {
        if (!setDefaultAction(args[0], actionName)) {
            err << "failed to set default action " << actionName
                << " for a class " << args[0] << endl;
            return 6;
        }
        break;
    }
    case PrintForFile:
    case InvokeForFile: {
        QList<Action> actions = Action::actionsForFile(QUrl(args[0]));
        foreach (const Action& action, actions) {
            if (todo == PrintForFile) {
                if (use_l10n)
                    out << action.localizedName() << endl;
                else
                    out << action.name() << endl;
            } else if (todo == InvokeForFile && actionName == action.name()) {
                action.trigger();
                return 0;
            }
        }
        if (todo == InvokeForFile) {
            err << "action '" << actionName << "'is not applicable\n";
            return 3;
        }
        break;
    }
    case LaunchFile: {
        Action::defaultActionForFile(QUrl(args[0])).trigger();
        return 0;
    }
    }
    return 0;
}
