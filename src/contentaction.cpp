#include "contentaction.h"

ContentAction::ContentAction()
{
    d = new ContentActionPrivate();
}

ContentAction::ContentAction(const ContentAction& other)
{
    d = new ContentActionPrivate();
    *d = *other.d;
}

ContentAction::~ContentAction()
{
    delete d;
    d = 0;
}

void ContentAction::trigger()
{
}

void ContentAction::setAsDefault()
{
}

bool ContentAction::isDefault()
{
    return false;
}

bool ContentAction::canBeDefault()
{
    return false;
}

ContentAction ContentAction::defaultAction(const QString& uri)
{
    return defaultAction(QStringList() << uri);
}

ContentAction ContentAction::defaultAction(const QStringList& uris)
{
    return ContentAction();
}

QList<ContentAction> ContentAction::actions(const QString& uri)
{
    return actions(QStringList() << uri);
}

QList<ContentAction> ContentAction::actions(const QStringList& uris)
{
    return QList<ContentAction>();
}


