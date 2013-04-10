#include <unistd.h>
#include <stdio.h>

#include <contentaction.h>

#include <QGraphicsGridLayout>
#include <QDebug>

#include <MApplication>
#include <MApplicationPage>
#include <MApplicationWindow>
#include <MLabel>
#include <MButton>
#include <MLocale>

static QString text("Hello, please reach me at foo@example.com\n"
                    "or call +1 555 23231.  But never send mail to\n"
                    "spamtrap@here.you.go because we'll call 911.");

class Us: public MWidget {
    Q_OBJECT
public:
    Us();
    ~Us();

private Q_SLOTS:
    void doHilite();
    void doUnHilite();

private:
    MLabel *textLabel;
    MButton *hiliteButton;
    MButton *unhiliteButton;
};

Us::Us() :
    textLabel(new MLabel(text, this)),
    hiliteButton(new MButton("hilite", this)),
    unhiliteButton(new MButton("unhilite", this))
{
    QGraphicsGridLayout *layout = new QGraphicsGridLayout(this);
    layout->addItem(hiliteButton, 0, 0);
    layout->addItem(unhiliteButton, 0, 1);
    layout->addItem(textLabel, 1, 0, 1, 2);
    textLabel->setWordWrap(true);
    connect(hiliteButton, SIGNAL(clicked()), this, SLOT(doHilite()));
    connect(unhiliteButton, SIGNAL(clicked()), this, SLOT(doUnHilite()));
}

Us::~Us()
{ }

void Us::doHilite()
{
    ContentAction::highlightLabel(textLabel);
}

void Us::doUnHilite()
{
    ContentAction::dehighlightLabel(textLabel);
}

int main(int argc, char **argv)
{
    MApplication app(argc, argv);
    MApplicationWindow window;
    MApplicationPage page;
    MLocale locale;

    if (!isatty(0))
        text = QTextStream(stdin).readAll();
    // MLabel doesn't wordwrap unless it's richtext...
    text.prepend("<span></span>");

    page.setCentralWidget(new Us());

    window.show();
    page.appear();
    return app.exec();
}
#include "hldemo.moc"
