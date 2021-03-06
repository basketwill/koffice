/* This file is part of the KDE project
 *
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2009 Thomas Zander <zander@kde.org>
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

#include "KShapePainter.h"

#include "KCanvasBase.h"
#include "KShapeManager.h"
#include "KShapeManagerPaintingStrategy.h"
#include "KShape.h"
#include "KViewConverter.h"
#include "KShapeBorderBase.h"
#include "KShapeGroup.h"
#include "KShapeContainer.h"

#include <KUnit.h>

#include <QtGui/QImage>
#include <QtGui/QPainter>

class SimpleCanvas : public KCanvasBase
{
public:
    SimpleCanvas()
        : KCanvasBase(0), m_shapeManager(new KShapeManager(this))
    {
    }

    ~SimpleCanvas()
    {
        delete m_shapeManager;
    }

    virtual void gridSize(qreal *horizontal, qreal *vertical) const
    {
        if (horizontal)
            *horizontal = 0;
        if (vertical)
            *vertical = 0;
    };

    virtual bool snapToGrid() const
    {
        return false;
    }

    virtual void addCommand(QUndoCommand *)
    {
    }

    virtual KShapeManager *shapeManager() const
    {
        return m_shapeManager;
    }

    virtual void updateCanvas(const QRectF&)
    {
    }

    virtual KToolProxy *toolProxy() const
    {
        return 0;
    }

    virtual const KViewConverter *viewConverter() const
    {
        return 0;
    }

    virtual QWidget *canvasWidget()
    {
        return 0;
    }

    virtual const QWidget *canvasWidget() const
    {
        return 0;
    }

    virtual KUnit unit() const
    {
        return KUnit(KUnit::Point);
    }

    virtual void updateInputMethodInfo() {}

private:
    KShapeManager *m_shapeManager;
};

class KShapePainter::Private
{
public:
    Private()
        : canvas(new SimpleCanvas())
    {
    }

    ~Private() { delete canvas; }
    SimpleCanvas * canvas;
};

KShapePainter::KShapePainter(KShapeManagerPaintingStrategy *strategy)
    : d(new Private())
{
    if (strategy) {
        strategy->setShapeManager(d->canvas->shapeManager());
        d->canvas->shapeManager()->setPaintingStrategy(strategy);
    }
}

KShapePainter::~KShapePainter()
{
    delete d;
}

void KShapePainter::setShapes(const QList<KShape*> &shapes)
{
    d->canvas->shapeManager()->setShapes(shapes, KShapeManager::AddWithoutRepaint);
}

void KShapePainter::paint(QPainter &painter, KViewConverter &converter)
{
    foreach (KShape *shape, d->canvas->shapeManager()->shapes()) {
        shape->waitUntilReady(converter, false);
    }

    d->canvas->shapeManager()->paint(painter, converter, true);
}

void KShapePainter::paint(QPainter &painter, const QRect &painterRect, const QRectF &documentRect)
{
    if (documentRect.width() == 0.0f || documentRect.height() == 0.0f)
        return;

    KViewConverter converter;
    // calculate the painter destination rectangle size in document coordinates
    QRectF paintBox = converter.viewToDocument(QRectF(QPointF(), painterRect.size()));

    // compute the zoom factor based on the bounding rects in document coordinates
    // so that the content fits into the image
    qreal zoomW = paintBox.width() / documentRect.width();
    qreal zoomH = paintBox.height() / documentRect.height();
    qreal zoom = qMin(zoomW, zoomH);

    // now set the zoom into the zoom handler used for painting the shape
    converter.setZoom(zoom);

    painter.save();

    // initialize painter
    painter.setPen(QPen(Qt::NoPen));
    painter.setBrush(Qt::NoBrush);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setClipRect(painterRect.adjusted(-1,-1,1,1));

    // convert document rectangle to view coordinates
    QRectF zoomedBound = converter.documentToView(documentRect);
    // calculate offset between painter rectangle and converted document rectangle
    QPointF offset = QPointF(painterRect.center()) - zoomedBound.center();
    // center content in painter rectangle
    painter.translate(offset.x(), offset.y());

    // finally paint the shapes
    paint(painter, converter);

    painter.restore();
}

void KShapePainter::paint(QImage &image)
{
    if (image.isNull())
        return;

    QPainter painter(&image);

    paint(painter, image.rect(), contentRect());
}

QRectF KShapePainter::contentRect()
{
    QRectF bound;
    foreach (KShape *shape, d->canvas->shapeManager()->shapes()) {
        if (!shape->isVisible(true))
            continue;
        if (dynamic_cast<KShapeGroup*>(shape))
            continue;

        QRectF shapeRect = shape->boundingRect();

        if (bound.isEmpty())
            bound = shapeRect;
        else
            bound = bound.united(shapeRect);
    }
    return bound;
}
