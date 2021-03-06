/* This file is part of the KDE project

   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006 Thomas Zander <zander@kde.org>

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
 * Boston, MA 02110-1301, USA.
*/

#ifndef K_SHAPE_RUBBER_SELECT_STRATEGY_H
#define K_SHAPE_RUBBER_SELECT_STRATEGY_H

#include "KInteractionStrategy.h"

#include <QRectF>

#include "flake_export.h"

class KToolBase;
class KShapeRubberSelectStrategyPrivate;

/**
 * Implement the rubber band selection of flake objects.
 */
class FLAKE_EXPORT KShapeRubberSelectStrategy : public KInteractionStrategy
{
public:
    /**
     * Constructor that initiates the rubber select.
     * A rubber select is basically rectangle area that the user drags out
     * from @p clicked to a point later provided in the handleMouseMove() continuously
     * showing a semi-transarant 'rubber-mat' over the objects it is about to select.
     * @param tool the parent tool which controls this strategy
     * @param clicked the initial point that the user depressed (in pt).
     * @param useSnapToGrid use the snap-to-grid settings while doing the rubberstamp.
     */
    KShapeRubberSelectStrategy(KToolBase *tool, const QPointF &clicked, bool useSnapToGrid = false);

    virtual void paint(QPainter &painter, const KViewConverter &converter);
    virtual void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers);
    virtual QUndoCommand *createCommand(QUndoCommand *parent = 0);
    virtual void finishInteraction(Qt::KeyboardModifiers modifiers);

protected:
    /// constructor
    KShapeRubberSelectStrategy(KShapeRubberSelectStrategyPrivate &);

private:
    Q_DECLARE_PRIVATE(KShapeRubberSelectStrategy)
};

#endif /* K_SHAPE_RUBBER_SELECT_STRATEGY_H */
