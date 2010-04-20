/*
 * Copyright (C) 2009, 2010 Nokia Corporation.
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

static const char help[] = \
"Usage: lca-tool [OPTIONS] MODE MODALCOMMAND URIS\n"
"       lca-tool [OPTIONS] OTHERCOMMAND ARGS\n"
"OPTION can be:\n"
"  --l10n              use localized names when printing actions\n"
"\n"
"MODE is one of:\n"
"  --tracker           URIS are representing objects stored in Tracker,\n"
"                      dispatched using Tracker-query based conditions\n"
"  --file              URI is a file (or other resource), dispatched based on\n"
"                      its content type\n"
"  --scheme            URIS are dispatched based on their scheme only\n"
"\n"
"In modal use, the following commands are available:\n"
"  --print             prints actions applicable to URIS\n"
"  --trigger ACTION    trigger ACTION with the given URIS\n"
"  --printdefault      print the default action for URIS\n"
"  --triggerdefault    trigger the default action for the given URIS\n"
"  --printmimes        print the (pseudo) mimetypes of URIS\n"
"\n"
"ACTION is the basename of the action's .desktop file (both when printing and\n"
"when invoking).\n"
"\n"
"Modeless commands are:\n"
"  --help                          print this text\n"
"  --actionsformime MIME           print actions for the given mimetype\n"
"  --mimedefault MIME              print the default action for the given\n"
"                                  mimetype\n"
"  --setmimedefault MIME ACTION    set ACTION as default for the given mimetype\n"
"  --resetmimedefault MIME         remove the user-defined default from the given mimetype\n"
"\n"
"Return values:\n"
"  0   success\n"
"  1   not enough arguments\n"
"  2   problems with the arguments\n"
"  3   triggered an action not applicable to the given URIS\n"
"  4   no default action exists for the given URIS\n"
"\n"
"Examples:\n"
"  $ lca-tool --tracker --triggerdefault urn:1246934-4213\n"
"  $ lca-tool --file --print file://$HOME/plaintext\n"
"  $ lca-tool --scheme --triggerdefault mailto:someone@example.com\n"
"  $ lca-tool --setmimedefault image/jpeg imageviewer\n";

enum UriMode {
    NoMode = 0,
    TrackerMode,
    FileMode,
    SchemeMode,
};

enum ActionToDo {
    Nothing,

    PrintHelp,

    PrintActions,
    TriggerAction,
    PrintDefaultAction,
    TriggerDefaultAction,
    PrintMimes,

    PrintActionsForMime,
    PrintMimeDefault,
    SetMimeDefault,
    ResetMimeDefault,
};

#define NEEDARG(errmsg)                         \
    do {                                        \
        if (args.isEmpty()) {                   \
            err << (errmsg) << endl;            \
            return 2;                           \
        }                                       \
    } while (0)

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QTextStream err(stderr), out(stdout);

    if (argc == 1) {
        err << help;
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

    UriMode mode = NoMode;
    ActionToDo todo = Nothing;
    bool use_l10n = false;
    QString actionName, mime;

    while (!args.isEmpty()) {
        UriMode newmode = NoMode;
        QString arg = args.takeFirst();

        if (!arg.startsWith("-"))
            break;
        // misc options
        if (arg == "--l10n") {
            use_l10n = true;
            continue;
        }
        // modes
        if (arg == "--tracker")
            newmode = TrackerMode;
        else if (arg == "--file")
            newmode = FileMode;
        else if (arg == "--scheme")
            newmode = SchemeMode;
        if (newmode != NoMode) {
            if (mode != NoMode) {
                err << "only a single MODE may be specified" << endl;
                return 2;
            }
            mode = newmode;
            newmode = NoMode;
            continue;
        }

        // modeless actions
        if (arg == "--help") {
            err << help;
            return 0;
        }
        else if (arg == "--actionsformime") {
            todo = PrintActionsForMime;
            NEEDARG("a MIME must be given when using --actionsformime");
            mime = args.takeFirst();
        }
        else if (arg == "--mimedefault") {
            todo = PrintMimeDefault;
            NEEDARG("a MIME must be given when using --mimedefault");
            mime = args.takeFirst();
        }
        else if (arg == "--setmimedefault") {
            todo = SetMimeDefault;
            NEEDARG("a MIME must be given when using --setmimedefault");
            mime = args.takeFirst();
            NEEDARG("an ACTION also must be given when using --setmimedefault");
            actionName = args.takeFirst();
        }
        else if (arg == "--resetmimedefault") {
            todo = ResetMimeDefault;
            NEEDARG("a MIME must be given when using --setmimedefault");
            mime = args.takeFirst();
        }
        // modal actions
        else if (arg == "--print") {
            todo = PrintActions;
        }
        else if (arg == "--trigger") {
            todo = TriggerAction;
            NEEDARG("an ACTION must be given when using --trigger");
            actionName = args.takeFirst();
        }
        else if (arg == "--printdefault") {
            todo = PrintDefaultAction;
        }
        else if (arg == "--triggerdefault") {
            todo = TriggerDefaultAction;
        }
        else if (arg == "--printmimes") {
            todo = PrintMimes;
        }
        else {
            err << "Unknown option " << arg << endl;
            return 2;
        }
        // bail out if we know what to do
        if (todo != Nothing)
            break;
    }

    // handle modeless actions first
    switch (todo) {
    case PrintActionsForMime:
        foreach (const Action& a, actionsForMime(mime)) {
            out << a.name() << endl;
        }
        return 0;
        break;
    case PrintMimeDefault:
        out << Internal::defaultAppForContentType(mime).remove(".desktop") << endl;
        return 0;
        break;
    case SetMimeDefault:
        setMimeDefault(mime, actionName);
        return 0;
        break;
    case ResetMimeDefault:
        resetMimeDefault(mime);
        return 0;
        break;
    default:
        break;
    }

    // then modals; they need arguments
    if (args.isEmpty()) {
        err << "option needs more arguments" << endl;
        return 2;
    }

    // get the Action:s and the default Action based on the mode
    QList<Action> actions;
    Action defAction;
    switch (mode) {
    case TrackerMode:
        actions = Action::actions(args);
        defAction = Action::defaultAction(args);
        break;
    case FileMode:
        actions = Action::actionsForFile(QUrl(args[0]));
        defAction = Action::defaultActionForFile(QUrl(args[0]));
        break;
    case SchemeMode:
        actions = Action::actionsForScheme(args[0]);
        defAction = Action::defaultActionForScheme(args[0]);
        break;
    default:
        break;
    }

    switch (todo) {
    case PrintActions:
        foreach (const Action& a, actions) {
            out << (use_l10n ? a.localizedName() : a.name()) << endl;
        }
        break;
    case TriggerAction:
        foreach (const Action& a, actions) {
            if (a.name() == actionName) {
                a.trigger();
                return 0;
            }
        }
        err << actionName << "is not applicable" << endl;
        return 3;
        break;
    case PrintDefaultAction:
        out << (use_l10n ? defAction.localizedName() : defAction.name()) << endl;
        break;
    case TriggerDefaultAction:
        if (!defAction.isValid()) {
            err << "no default action for the given URIs" << endl;
            return 4;
        }
        defAction.trigger();
        return 0;
        break;
    case PrintMimes: {
        switch (mode) {
        case TrackerMode:
            foreach (const QString& mime, mimeForTrackerObject(args[0])) {
                out << mime << endl;
            }
            break;
        case FileMode:
            out << mimeForFile(args[0]) << endl;
            break;
        case SchemeMode:
            out << mimeForScheme(args[0]) << endl;
            break;
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
    return 0;
}
