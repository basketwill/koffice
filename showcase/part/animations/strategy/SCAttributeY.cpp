/* This file is part of the KDE project
 * Copyright (C) 2010 Benjamin Port <port.benjamin@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (  at your option ) any later version.
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

#include "SCAttributeY.h"
#include "../SCAnimationCache.h"
#include "KShape.h"
#include "../SCShapeAnimation.h"

#include "kdebug.h"

SCAttributeY::SCAttributeY() : SCAnimationAttribute("y")
{
}

void SCAttributeY::updateCache(SCAnimationCache *cache, SCShapeAnimation *shapeAnimation, qreal value)
{
    KShape *shape = shapeAnimation->shape();
    QTransform transform;
    value = value * cache->pageSize().height();
    value = value - shape->position().y();
    value = value * cache->zoom();
    transform.translate(0, value);
    cache->update(shape, shapeAnimation->textBlockData(), "transform", transform);
}

void SCAttributeY::initCache(SCAnimationCache *animationCache, int step, SCShapeAnimation * shapeAnimation, qreal startValue, qreal endValue)
{
    KShape * shape = shapeAnimation->shape();
    qreal v1 = (startValue * animationCache->pageSize().height() - shape->position().y()) * animationCache->zoom();
    qreal v2 = (endValue * animationCache->pageSize().height() - shape->position().y()) * animationCache->zoom();
    animationCache->init(step, shape, shapeAnimation->textBlockData(), "transform", QTransform().translate(0, v1));
    animationCache->init(step + 1, shape, shapeAnimation->textBlockData(), "transform", QTransform().translate(0, v2));
}
