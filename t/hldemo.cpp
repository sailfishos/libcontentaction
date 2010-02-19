#include <unistd.h>
#include <stdio.h>

#include <contentaction.h>
#include <contentaction-dui.h>

#include <QGraphicsGridLayout>
#include <QDebug>

#include <DuiApplication>
#include <DuiApplicationPage>
#include <DuiApplicationWindow>
#include <DuiLabel>
#include <DuiButton>
#include <DuiLocale>

static QString text("Hello, please reach me at foo@example.com\n"
                    "or call +1 555 23231.  But never send mail to\n"
                    "spamtrap@here.you.go because we'll call 911.");

class Us: public DuiWidget {
    Q_OBJECT
public:
    Us();
    ~Us();

private slots:
    void doHilite();

private:
    DuiLabel *textLabel;
    DuiButton *hiliteButton;
};

Us::Us() :
    textLabel(new DuiLabel(text, this)),
    hiliteButton(new DuiButton("hilite me", this))
{
    QGraphicsGridLayout *layout = new QGraphicsGridLayout(this);
    layout->addItem(hiliteButton, 0, 0);
    layout->addItem(textLabel, 1, 0, 1, 2);
    connect(hiliteButton, SIGNAL(clicked()), this, SLOT(doHilite()));
}

Us::~Us()
{ }

void Us::doHilite()
{
    ContentAction::Dui::highlightLabel(textLabel);
}

int main(int argc, char **argv)
{
    DuiApplication app(argc, argv);
    DuiApplicationWindow window;
    DuiApplicationPage page;
    DuiLocale locale;

    ContentAction::Action::installTranslators(locale.name());
    if (!isatty(0))
        text = QTextStream(stdin).readAll();
    // Need to trick DuiLabel into html mode...
    text.prepend("<span></span>");

    page.setCentralWidget(new Us());

    window.show();
    page.appearNow();
    return app.exec();
}
#include "hldemo.moc"
