/* This file is part of the KDE project

   Copyright (C) 2007,2009 Thorsten Zachmann <zachmann@kde.org>

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

#ifndef KSHAPEMANAGERPAINTINGSTRATEGY_H
#define KSHAPEMANAGERPAINTINGSTRATEGY_H

#include "flake_export.h"

class KShapeManager;
class KShape;
class KViewConverter;
class QPainter;
class QRectF;

/**
 * This implements the painting strategy for the KShapeManager
 *
 * This is done to make it possible to have e.g. animations in showcase.
 *
 * This class implements the default strategy which is normally used.
 */
class FLAKE_EXPORT KShapeManagerPaintingStrategy
{
public:
    KShapeManagerPaintingStrategy(KShapeManager *shapeManager);
    virtual ~KShapeManagerPaintingStrategy();

    /**
     * Paint the shape
     *
     * @param shape the shape to paint
     * @param painter the painter to paint to.
     * @param converter to convert between document and view coordinates.
     * @param forPrint if true, make sure only actual content is drawn and no decorations.
     */
    virtual void paint(KShape *shape, QPainter &painter, const KViewConverter &converter, bool forPrint);

    /**
     * Adapt the rect the shape occupies
     *
     * @param rect rect which will be updated to give the rect the shape occupies.
     */
    virtual void adapt(KShape *shape, QRectF &rect);

    /**
     * Set the shape manager
     *
     * This is needed in case you cannot set the shape manager when creating the paiting strategy.
     * It needs to be set before you paint otherwise nothing will be painted.
     *
     * @param shapeManager The shape manager to use in the painting startegy
     */
    void setShapeManager(KShapeManager *shapeManager);

protected:
    KShapeManager *shapeManager();

private:
    class Private;
    Private * const d;
};

#endif /* KOSHAPEMANAGERPAINTINGSTRATEGY_H */
