/* This file is part of the KDE project
 * Copyright (C) 2010 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef SCANIMATIONBASE_H
#define SCANIMATIONBASE_H

#include <QAbstractAnimation>
#include "SCAnimationData.h"

class KXmlElement;
class KShapeLoadingContext;
class KShapeSavingContext;
class KShape;
class KTextBlockData;
class SCAnimationCache;
class SCShapeAnimation;
class KoPASavingContext;

class SCAnimationBase : public QAbstractAnimation, SCAnimationData
{
public:
    SCAnimationBase(SCShapeAnimation *shapeAnimation);
    virtual ~SCAnimationBase();
    virtual bool loadOdf(const KXmlElement &element, KShapeLoadingContext &context);
    virtual bool saveOdf(KoPASavingContext &paContext) const = 0;

    virtual int duration() const;
    virtual void init(SCAnimationCache *animationCache, int step) = 0;
    int animationDuration() const;
    virtual bool saveAttribute(KoPASavingContext &paContext) const;
protected:
    virtual void updateCurrentTime(int currentTime);
    virtual void next(int currentTime) = 0;
    void updateCache(const QString &id, const QVariant &value);


    SCShapeAnimation *m_shapeAnimation; // we could also use the group() but that would mean we need to cast all the time
    SCAnimationCache * m_animationCache;
    int m_begin; // in milliseconds
    int m_duration; // in milliseconds
};

#endif /* SCANIMATIONBASE_H */
