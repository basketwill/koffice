/* This file is part of the KDE project
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KInlineObjectRegistry.h"
#include "KInlineObjectFactoryBase.h"
#include "InsertVariableAction_p.h"

#include <KCanvasBase.h>
#include <KInlineObject.h>
#include <KXmlReader.h>
#include <KoPluginLoader.h>

#include <KDebug>
#include <KGlobal>

class KInlineObjectRegistry::Private
{
public:
    void insertFactory(KInlineObjectFactoryBase *factory);
    void init(KInlineObjectRegistry *q);

    QHash<QPair<QString, QString>, KInlineObjectFactoryBase *> factories;
};

void KInlineObjectRegistry::Private::init(KInlineObjectRegistry *q)
{
    KoPluginLoader::PluginsConfig config;
    config.whiteList = "TextInlinePlugins";
    config.blacklist = "TextInlinePluginsDisabled";
    config.group = "koffice";
    KoPluginLoader::instance()->load(QString::fromLatin1("KOffice/Text-InlineObject"),
                                     QString::fromLatin1("[X-KOdfText-MinVersion] <= 0"), config);

    for (KGenericRegistry<KInlineObjectFactoryBase*>::const_iterator it = q->constBegin(); it != q->constEnd(); ++it) {
        KInlineObjectFactoryBase *factory = it.value();
        QString nameSpace = factory->odfNameSpace();
        if (nameSpace.isEmpty() || factory->odfElementNames().isEmpty()) {
            kDebug(32500) << "Variable factory" << factory->id() << " does not have odfNameSpace defined, ignoring";
        } else {
            foreach (const QString &elementName, factory->odfElementNames()) {
                factories.insert(QPair<QString, QString>(nameSpace, elementName), factory);

                kDebug(32500) << "Inserting variable factory" << factory->id() << " for"
                    << nameSpace << ":" << elementName;
            }
        }
    }
}

KInlineObjectRegistry* KInlineObjectRegistry::instance()
{
    K_GLOBAL_STATIC(KInlineObjectRegistry, s_instance)
    if (!s_instance.exists()) {
        s_instance->d->init(s_instance);
    }
    return s_instance;
}

QList<QAction*> KInlineObjectRegistry::createInsertVariableActions(KCanvasBase *host) const
{
    QList<QAction*> answer;
    foreach (const QString &key, keys()) {
        KInlineObjectFactoryBase *factory = value(key);
        if (factory->type() == KInlineObjectFactoryBase::TextVariable) {
            foreach (const KoInlineObjectTemplate &templ, factory->templates()) {
                answer.append(new InsertVariableAction(host, factory, templ));
            }
#ifndef NDEBUG
           if (factory->templates().isEmpty()) {
                kWarning(32500) << "Variable factory" << factory->id() << "has no templates, skipping.";
           }
#endif
        }
    }
    return answer;
}

KInlineObject *KInlineObjectRegistry::createFromOdf(const KXmlElement &element, KShapeLoadingContext &context) const
{
    kDebug(32500) << "Going to check for" << element.namespaceURI() << ":" << element.tagName();

    KInlineObjectFactoryBase *factory = d->factories.value(
            QPair<QString, QString>(element.namespaceURI(), element.tagName()));
    if (factory == 0)
        return 0;

    KInlineObject *object = factory->createInlineObject(0);
    if (object) {
        object->loadOdf(element, context);
    }

    return object;
}

KInlineObjectRegistry::~KInlineObjectRegistry()
{
    delete d;
}

KInlineObjectRegistry::KInlineObjectRegistry()
        : d(new Private())
{
}

#include <KInlineObjectRegistry.moc>
