#ifndef CONTENTACTION_H
#define CONTENTACTION_H

#include <QList>
#include <QString>
#include <QStringList>

struct ContentActionPrivate
{
    QString uri;
    QStringList classes;
    QString service; // <service fw interface>.<method>
};

class ContentAction
{
public:
    ContentAction(const ContentAction& other);
    ~ContentAction();

    void trigger();
    void setAsDefault();
    bool isDefault();
    bool canBeDefault();

    static ContentAction defaultAction(const QString& uri);
    static ContentAction defaultAction(const QStringList& uris);

    static QList<ContentAction> actions(const QString& uri);
    static QList<ContentAction> actions(const QStringList& uris);

private:
    ContentAction();
    ContentActionPrivate* d;
};

#endif
