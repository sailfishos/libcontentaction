/*
 * Reads text from the standard input, highlight rules from the usual place
 * (ie. xml files in $CONTENTACTION_ACTIONS) and then prints match results on
 * the standard output.  If the terminal is a tty, the results are printed on
 * stderr, and on stdout a beautifully colored version of the text is shown.
 */
#include <stdio.h>
#include <unistd.h>
#include <contentaction.h>
#include <QDebug>

using namespace ContentAction;

QDebug operator<<(QDebug dbg, const Match& m) {
    dbg.nospace() << "match at ("
                  << m.start << ", " << m.end << "): ";
    foreach (const Action& a, m.actions) {
        dbg.space() << a.name();
    }
    dbg.nospace() << "\n";
    return dbg;
}

QDebug operator<<(QDebug dbg, const QList<Match>& ms) {
    foreach (const Match& m, ms) {
        dbg.space() << m;
    }
    return dbg;
}

int main(void)
{
    QTextStream out(isatty(1) ? stderr : stdout);
    QTextStream textout(isatty(1) > 0 ? stdout : fopen("/dev/null", "w"));
    QTextStream in(stdin);
    QString text = in.readAll();

    QList<Match> ms = Action::highlight(text);
    foreach (const Match& m, ms) {
        QStringList actions;
        foreach (const Action& a, m.actions)
            actions << a.name();
        out << QString("%1 %2 '%3' %4\n").arg(QString::number(m.start),
                                              QString::number(m.end),
                                              text.mid(m.start, m.end - m.start),
                                              actions.join(" "));
    }
    QString hltext(text);
    if (isatty(1)) {
        qSort(ms.begin(), ms.end());
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
        foreach (const Match& m, ms) {
            hltext.insert(d + m.start, color[i]);
            d += color[i].length();
            i = (i + 1) % (sizeof(color) / sizeof(color[0]));
            hltext.insert(d + m.end, "\e[0m");
            d += 4;
        }
        textout << hltext;
    }
    return 0;
}
