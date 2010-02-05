#include <contentaction.h>
#include <contentaction-dui.h>

#include <QGraphicsGridLayout>
#include <QDebug>

#include <DuiApplication>
#include <DuiApplicationPage>
#include <DuiApplicationWindow>
#include <DuiLabel>
#include <DuiButton>

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
    textLabel(new DuiLabel("<b>need to trick the label into html mode</b>\n"
                           "this is some sample text email@address.com\n"
                           "+34241 number more 431", this)),
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

    page.setCentralWidget(new Us());

    window.show();
    page.appearNow();
    return app.exec();
}
#include "hldemo.moc"
