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
#include <QFileInfo>
#include <mlocale/MLocale>

using namespace ContentAction;
using namespace ContentAction::Internal;

static const char help[] = \
"Usage: lca-tool [OPTIONS] MODE MODALCOMMAND PARAMS\n"
"       lca-tool [OPTIONS] OTHERCOMMAND ARGS\n"
"OPTION can be:\n"
"  --l10n              use localized names when printing actions\n"
"\n"
"MODE is one of:\n"
"  --tracker           PARAMS are representing objects stored in Tracker,\n"
"                      dispatched using Tracker-query based conditions\n"
"  --file              PARAMS is a file (or other resource), dispatched based on\n"
"                      its content type\n"
"  --scheme            PARAMS are dispatched based on their scheme only\n"
"  --string            PARAMS are dispatched based on regexp matches\n"
"\n"
"In modal use, the following commands are available:\n"
"  --print             prints actions applicable to PARAMS\n"
"  --trigger ACTION    trigger ACTION with the given PARAMS\n"
"  --printdefault      print the default action for PARAMS\n"
"  --triggerdefault    trigger the default action for the given PARAMS\n"
"  --printmimes        print the (pseudo) mimetypes of PARAMS\n"
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
"  --highlight                     will read text from stdin and find actions for items in it\n"
"  --triggerdesktop DESKTOPFILE PARAMS   will launch the application defined by DESKTOPFILE with the given PARAMS\n"

"\n"
"Return values:\n"
"  0   success\n"
"  1   not enough arguments\n"
"  2   problems with the arguments\n"
"  3   triggered an action not applicable to the given URIS\n"
"  4   no default action exists for the given URIS\n"
"  5   desktop file not found\n"
"\n"
"Examples:\n"
"  $ lca-tool --tracker --triggerdefault urn:1246934-4213\n"
"  $ lca-tool --file --print file://$HOME/plaintext\n"
"  $ lca-tool --scheme --triggerdefault mailto:someone@example.com\n"
"  $ lca-tool --string --print \"myaddress@email.com\""
"  $ lca-tool --setmimedefault image/jpeg imageviewer\n"
"  $ lca-tool --highlight < myinput.txt\n"
"  $ lca-tool --triggerdesktop myapp.desktop param1 param2\n";

enum UriMode {
    NoMode = 0,
    TrackerMode,
    FileMode,
    SchemeMode,
    StringMode,
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
    Highlight,
    TriggerDesktop
};

#define NEEDARG(errmsg)                         \
    do {                                        \
        if (args.isEmpty()) {                   \
            err << (errmsg) << endl;            \
            return 2;                           \
        }                                       \
    } while (0)

// The highlighter part:

QDebug operator<<(QDebug dbg, const Match& m) {
    dbg.nospace() << "match at ("
                  << m.start << ", " << m.end << "): ";
    Q_FOREACH (const Action& a, m.actions) {
        dbg.space() << a.name();
    }
    dbg.nospace() << "\n";
    return dbg;
}

QDebug operator<<(QDebug dbg, const QList<Match>& ms) {
    Q_FOREACH (const Match& m, ms) {
        dbg.space() << m;
    }
    return dbg;
}

/*
 * Reads text from the standard input, highlight rules from the usual place
 * (ie. xml files in $CONTENTACTION_ACTIONS) and then prints match results on
 * the standard output.  If the terminal is a tty, the results are printed on
 * stderr, and on stdout a beautifully colored version of the text is shown.
 */
void doHighlight()
{
    QTextStream out(isatty(1) ? stderr : stdout);
    QTextStream textout(isatty(1) > 0 ? stdout : fopen("/dev/null", "w"));
    QTextStream in(stdin);
    QString text = in.readAll();

    QList<QPair<int, int> > highlights = Action::findHighlights(text);
    QList<QPair<int, int> >::const_iterator it = highlights.begin();
    while (it != highlights.end()) {
        QString highlight = text.mid(it->first, it->second);
        QStringList actions;
        Q_FOREACH (const Action& a, Action::actionsForString(highlight))
            actions << a.name();
        out << QString("%1 %2 '%3' %4\n").arg(QString::number(it->first),
                                              QString::number(it->first + it->second),
                                              highlight,
                                              actions.join(" "));
        ++it;
    }
    QString hltext(text);
    if (isatty(1)) {
        QString color[] = {
            "\e[1;37;41m",
            "\e[1;37;42m",
            "\e[1;37;43m",
            "\e[1;37;44m",
            "\e[1;37;45m",
            "\e[1;37;46m",
        };
        int i = 0;
        int d = 0;
        it = highlights.begin();
        while (it != highlights.end()) {
            hltext.insert(d + it->first, color[i]);
            d += color[i].length();
            i = (i + 1) % (sizeof(color) / sizeof(color[0]));
            hltext.insert(d + it->second, "\e[0m");
            d += 4;
            ++it;
        }
        textout << hltext;
    }
}

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
        Q_FOREACH (const QString& p, QString::fromLocal8Bit(l10npaths).split(':')) {
            qDebug() << "adding path:" << p;
            ML10N::MLocale::addTranslationPath(p);
        }
    } else {
        ML10N::MLocale::addTranslationPath("/usr/share/l10n/meegotouch");
    }

    QStringList args;
    for (int i = 1; i < argc; ++i)
        args << QString::fromLocal8Bit(argv[i]);

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
        else if (arg == "--string")
            newmode = StringMode;
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
        else if (arg == "--highlight") {
            todo = Highlight;
        }
        else if (arg == "--triggerdesktop") {
            todo = TriggerDesktop;
            NEEDARG("a DESKTOPFILE must be given when using --triggerdesktop");
            actionName = args.takeFirst();
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
        Q_FOREACH (const Action& a, actionsForMime(mime)) {
            out << a.name() << endl;
        }
        return 0;
        break;
    case PrintMimeDefault:
        out << defaultActionForMime(mime).name() << endl;
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
    case Highlight:
        doHighlight();
        return 0;
        break;
    case TriggerDesktop:
    {
        Action a = Action::launcherAction(actionName, args);
        if (!a.isValid())
            return 5;
        a.triggerAndWait();
        return 0;
    }
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
        if (args.size() > 1) {
            actions = Action::actions(args);
            defAction = Action::defaultAction(args);
        } else {
            actions = Action::actions(args[0]);
            defAction = Action::defaultAction(args[0]);
        }
        break;
    case FileMode:
    {
        QList<QUrl> uris;
        for (int i = 0; i < args.size(); i++) {
            if (args[i].startsWith ("file:")) {
                uris << QUrl::fromEncoded(args[i].toLocal8Bit());
            } else {
                uris << QUrl::fromLocalFile(args[i]);
            }
        }
        if (uris.size() == 1)
          {
            defAction = Action::defaultActionForFile(uris[0]);
            actions = Action::actionsForFile(uris[0]);
          }
        else
          {
            QString mimeType;
            if (uris.size() > 0)
                mimeType = mimeForFile (uris[0]);
            defAction = Action::defaultActionForFile(uris, mimeType);
            actions = Action::actionsForFile(uris, mimeType);
          }
    }
    break;
    case SchemeMode:
        actions = Action::actionsForScheme(args[0]);
        defAction = Action::defaultActionForScheme(args[0]);
        break;
    case StringMode:
        actions = Action::actionsForString(args[0]);
        defAction = Action::defaultActionForString(args[0]);
        break;
    default:
        break;
    }

    switch (todo) {
    case PrintActions:
        Q_FOREACH (const Action& a, actions) {
            out << (use_l10n ? a.localizedName() : a.name()) << endl;
        }
        break;
    case TriggerAction:
        Q_FOREACH (const Action& a, actions) {
            if (a.name() == actionName) {
                a.triggerAndWait();
                return 0;
            }
        }
        err << actionName << " is not applicable" << endl;
        return 3;
        break;
    case PrintDefaultAction:
        out << (use_l10n ? defAction.localizedName() : defAction.name()) << endl;
        break;
    case TriggerDefaultAction:
        if (!defAction.isValid()) {
            err << "no default action for the given PARAMS" << endl;
            return 4;
        }
        defAction.triggerAndWait();
        return 0;
        break;
    case PrintMimes: {
        switch (mode) {
        case TrackerMode:
            Q_FOREACH (const QString& mime, mimeForTrackerObject(args[0])) {
                out << mime << endl;
            }
            break;
        case FileMode:
            out << mimeForFile(args[0]) << endl;
            break;
        case SchemeMode:
            out << mimeForScheme(args[0]) << endl;
            break;
        case StringMode:
            Q_FOREACH (const QString& mime, mimeForString(args[0])) {
                out << mime << endl;
            }
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
