/*
 * Copyright (C) 2015 Jolla Ltd.
 *
 * Contact: Aaron McCarthy <aaron.mccarthy@jollamobile.com>
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

#include "plugin.h"
#include "declarativecontentaction.h"

#include <QtQml/qqml.h>

namespace
{

QObject *content_action(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    return new DeclarativeContentAction;
}

}

void DeclarativePlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    Q_UNUSED(engine)
    Q_UNUSED(uri)

    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.nemomobile.contentaction"));
}

void DeclarativePlugin::registerTypes(const char *uri)
{
    // @uri org.nemomobile.contentaction
    qmlRegisterSingletonType<DeclarativeContentAction>(uri, 1, 0, "ContentAction", content_action);

}
