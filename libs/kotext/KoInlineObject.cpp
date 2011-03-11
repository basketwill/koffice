/* This file is part of the KDE project
 * Copyright (C) 2006-2011 Thomas Zander <zander@kde.org>
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

#include "KoInlineObject.h"
#include "KoInlineObject_p.h"
#include "KoTextDocumentLayout.h"
#include "KoTextShapeData.h"
#include "KoShapeSavingContext.h"
#include "KoInlineTextObjectManager.h"
#include "KoTextInlineRdf.h"

#include <KoShape.h>

#include <kdebug.h>
#include <QDebug>

QDebug KoInlineObjectPrivate::printDebug(QDebug dbg) const
{
    dbg.nospace() << "KoInlineObject ManagerId: " << id;
    return dbg.space();
}

void KoInlineObjectPrivate::callbackPositionChanged()
{
    Q_Q(KoInlineObject);
    q->positionChanged();
}

KoInlineObjectPrivate::~KoInlineObjectPrivate()
{
    delete rdf;
}


//////////////////////////////////////

KoInlineObject::KoInlineObject(bool propertyChangeListener)
        : d_ptr(new KoInlineObjectPrivate(this))
{
    Q_D(KoInlineObject);
    d->propertyChangeListener = propertyChangeListener;
}

KoInlineObject::KoInlineObject(KoInlineObjectPrivate &priv, bool propertyChangeListener)
    : d_ptr(&priv)
{
    Q_D(KoInlineObject);
    d->propertyChangeListener = propertyChangeListener;
}

KoInlineObject::~KoInlineObject()
{
    if (d_ptr->manager) {
        d_ptr->manager->removeInlineObject(this);
    }
    delete d_ptr;
    d_ptr = 0;
}

void KoInlineObject::setManager(KoInlineTextObjectManager *manager)
{
    Q_D(KoInlineObject);
    d->manager = manager;
}

KoInlineTextObjectManager *KoInlineObject::manager()
{
    Q_D(KoInlineObject);
    return d->manager;
}

void KoInlineObject::propertyChanged(Property key, const QVariant &value)
{
    Q_UNUSED(key);
    Q_UNUSED(value);
}

int KoInlineObject::id() const
{
    Q_D(const KoInlineObject);
    return d->id;
}

void KoInlineObject::setId(int id)
{
    Q_D(KoInlineObject);
    d->id = id;
}

bool KoInlineObject::propertyChangeListener() const
{
    Q_D(const KoInlineObject);
    return d->propertyChangeListener;
}

//static
KoShape * KoInlineObject::shapeForPosition(const QTextDocument *document, int position)
{
    KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(document->documentLayout());
    if (lay == 0)
        return 0;
    return lay->shapeForPosition(position);
}

QDebug operator<<(QDebug dbg, const KoInlineObject *o)
{
    return o->d_func()->printDebug(dbg);
}

void KoInlineObject::setInlineRdf(KoTextInlineRdf* rdf)
{
    Q_D(KoInlineObject);
    d->rdf = rdf;
}

KoTextInlineRdf* KoInlineObject::inlineRdf() const
{
    Q_D(const KoInlineObject);
    return d->rdf;
}

void KoInlineObject::setDocument(QTextDocument *doc)
{
    Q_D(KoInlineObject);
    if (d->document == doc)
        return;
    d->document = doc;
    d->positionInDocument = -1;
}

QTextDocument *KoInlineObject::document() const
{
    Q_D(const KoInlineObject);
    return d->document;
}

void KoInlineObject::setTextPosition(int pos)
{
    Q_D(KoInlineObject);
    if (d->positionInDocument == pos)
        return;
    d->positionInDocument = pos;
    positionChanged();
}

int KoInlineObject::textPosition() const
{
    Q_D(const KoInlineObject);
    return d->positionInDocument;
}

KoShape *KoInlineObject::shape() const
{
    Q_D(const KoInlineObject);
    return shapeForPosition(d->document, d->positionInDocument);
}

KoTextPage *KoInlineObject::page() const
{
    Q_D(const KoInlineObject);
    KoShape *shape = shapeForPosition(d->document, d->positionInDocument);
    if (shape == 0)
        return 0;
    KoTextShapeData *data = static_cast<KoTextShapeData*>(shape->userData());
    if (data == 0)
        return 0;
    return data->page();
}

void KoInlineObject::positionChanged()
{
}

KoInlineObjectPrivate *KoInlineObject::priv()
{
    Q_D(KoInlineObject);
    return d;
}
