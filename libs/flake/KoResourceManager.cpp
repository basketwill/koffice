/*
   Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
   Copyright (C) 2007, 2010-2011 Thomas Zander <zander@kde.org>
   Copyright (c) 2008 Carlos Licea <carlos.licea@kdemail.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */
#include "KoResourceManager.h"
#include "KoShape.h"
#include "KoLineBorder.h"

#include <QVariant>
#include <QMetaObject>
#include <KUndoStack>
#include <KDebug>

class KoResourceManager::Private
{
public:
    void fetchLazy(int key, const KoResourceManager *parent) {
        Q_ASSERT(lazyResources.contains(key));
        Slot slot = lazyResources.value(key);
        lazyResources.remove(key);

        KoResourceManager *rm = const_cast<KoResourceManager*>(parent);
        slot.object->metaObject()->invokeMethod(slot.object, slot.slot,
            Qt::DirectConnection, Q_ARG(KoResourceManager *, rm));
    }


    QHash<int, QVariant> resources;
    struct Slot {
        QObject *object;
        const char *slot;
    };
    QHash<int, Slot> lazyResources;
};

KoResourceManager::KoResourceManager(QObject *parent)
        : QObject(parent),
        d(new Private())
{
    setGrabSensitivity(3);
}

KoResourceManager::~KoResourceManager()
{
    delete d;
}

void KoResourceManager::setResource(int key, const QVariant &value)
{
    if (d->resources.contains(key)) {
        if (d->resources.value(key) == value)
            return;
        d->resources[key] = value;
    } else {
        d->resources.insert(key, value);
        d->lazyResources.remove(key);
    }
    emit resourceChanged(key, value);
}

QVariant KoResourceManager::resource(int key) const
{
    if (d->lazyResources.contains(key))
        d->fetchLazy(key, this);
    if (!d->resources.contains(key)) {
        QVariant empty;
        return empty;
    } else
        return d->resources.value(key);
}

void KoResourceManager::setResource(int key, KoShape *shape)
{
    QVariant v;
    v.setValue(shape);
    setResource(key, v);
}

void KoResourceManager::setResource(int key, const KUnit &unit)
{
    QVariant v;
    v.setValue(unit);
    setResource(key, v);
}

QColor KoResourceManager::colorResource(int key) const
{
    if (d->lazyResources.contains(key))
        d->fetchLazy(key, this);
    if (! d->resources.contains(key))
        return QColor();
    return resource(key).value<QColor>();
}

void KoResourceManager::setForegroundColor(const QColor &color)
{
    setResource(KoCanvasResource::ForegroundColor, color);
}

QColor KoResourceManager::foregroundColor() const
{
    return colorResource(KoCanvasResource::ForegroundColor);
}

void KoResourceManager::setBackgroundColor(const QColor &color)
{
    setResource(KoCanvasResource::BackgroundColor, color);
}

QColor KoResourceManager::backgroundColor() const
{
    return colorResource(KoCanvasResource::BackgroundColor);
}

KoShape *KoResourceManager::koShapeResource(int key) const
{
    if (d->lazyResources.contains(key))
        d->fetchLazy(key, this);
    if (! d->resources.contains(key))
        return 0;

    return resource(key).value<KoShape *>();
}

void KoResourceManager::setHandleRadius(int handleRadius)
{
    // do not allow arbitrary small handles
    if (handleRadius < 3)
        handleRadius = 3;
    setResource(KoCanvasResource::HandleRadius, QVariant(handleRadius));
}

int KoResourceManager::handleRadius() const
{
    if (d->resources.contains(KoCanvasResource::HandleRadius))
        return d->resources.value(KoCanvasResource::HandleRadius).toInt();
    return 3; // default value.
}

KUnit KoResourceManager::unitResource(int key) const
{
    if (d->lazyResources.contains(key))
        d->fetchLazy(key, this);
    return resource(key).value<KUnit>();
}

void KoResourceManager::setGrabSensitivity(int grabSensitivity)
{
    // do not allow arbitrary small handles
    if (grabSensitivity < 1)
        grabSensitivity = 1;
    setResource(KoCanvasResource::GrabSensitivity, QVariant(grabSensitivity));
}

int KoResourceManager::grabSensitivity() const
{
    return resource(KoCanvasResource::GrabSensitivity).toInt();
}

bool KoResourceManager::boolResource(int key) const
{
    if (d->lazyResources.contains(key))
        d->fetchLazy(key, this);
    if (! d->resources.contains(key))
        return false;
    return d->resources[key].toBool();
}

int KoResourceManager::intResource(int key) const
{
    if (d->lazyResources.contains(key))
        d->fetchLazy(key, this);
    if (! d->resources.contains(key))
        return 0;
    return d->resources[key].toInt();
}

QString KoResourceManager::stringResource(int key) const
{
    if (d->lazyResources.contains(key))
        d->fetchLazy(key, this);
    if (! d->resources.contains(key)) {
        QString empty;
        return empty;
    }
    return qvariant_cast<QString>(resource(key));
}

QSizeF KoResourceManager::sizeResource(int key) const
{
    if (d->lazyResources.contains(key))
        d->fetchLazy(key, this);
    if (! d->resources.contains(key)) {
        QSizeF empty;
        return empty;
    }
    return qvariant_cast<QSizeF>(resource(key));
}

bool KoResourceManager::hasResource(int key) const
{
    return d->resources.contains(key) || d->lazyResources.contains(key);
}

void KoResourceManager::clearResource(int key)
{
    if (! d->resources.contains(key))
        return;
    d->resources.remove(key);
    QVariant empty;
    emit resourceChanged(key, empty);
}

void KoResourceManager::setLazyResourceSlot(int key, QObject *object, const char *slot)
{
    if (d->resources.contains(key) || d->lazyResources.contains(key))
        return;
    KoResourceManager::Private::Slot target;
    target.object = object;
    target.slot = slot;
    d->lazyResources.insert(key, target);
}

KUndoStack *KoResourceManager::undoStack() const
{
    if (!hasResource(KoDocumentResource::UndoStack))
        return 0;
    return static_cast<KUndoStack*>(resource(KoDocumentResource::UndoStack).value<void*>());
}

void KoResourceManager::setUndoStack(KUndoStack *undoStack)
{
    QVariant variant;
    variant.setValue<void*>(undoStack);
    setResource(KoDocumentResource::UndoStack, variant);
}

KImageCollection *KoResourceManager::imageCollection() const
{
    if (!hasResource(KoDocumentResource::ImageCollection))
        return 0;
    return static_cast<KImageCollection*>(resource(KoDocumentResource::ImageCollection).value<void*>());
}

void KoResourceManager::setImageCollection(KImageCollection *ic)
{
    QVariant variant;
    variant.setValue<void*>(ic);
    setResource(KoDocumentResource::ImageCollection, variant);
}

KOdfDocumentBase *KoResourceManager::odfDocument() const
{
    if (!hasResource(KoDocumentResource::OdfDocument))
        return 0;
    return static_cast<KOdfDocumentBase*>(resource(KoDocumentResource::OdfDocument).value<void*>());
}

void KoResourceManager::setOdfDocument(KOdfDocumentBase *currentDocument)
{
    QVariant variant;
    variant.setValue<void*>(currentDocument);
    setResource(KoDocumentResource::OdfDocument, variant);
}

void KoResourceManager::setTextDocumentList(const QList<QTextDocument *> &allDocuments)
{
    QList<QVariant> list;
    foreach (QTextDocument *doc, allDocuments) {
        QVariant v;
        v.setValue<void*>(doc);
        list << v;
    }
    setResource(KoDocumentResource::TextDocuments, list);
}

QList<QTextDocument *> KoResourceManager::textDocumentList() const
{
    QList<QTextDocument*> answer;
    QVariant variant = resource(KoDocumentResource::TextDocuments);
    if (variant.isNull())
        return answer;
    QList<QVariant> list = qvariant_cast<QList<QVariant> >(variant);
    foreach (const QVariant &variant, list) {
        answer << static_cast<QTextDocument*>(variant.value<void*>());
    }
    return answer;
}

#include <KoResourceManager.moc>
