/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#include "KoInteractionStrategy.h"
#include "KoSelection.h"
#include "KoShapeManager.h"
#include "KoPointerEvent.h"
#include "KoShapeRubberSelectStrategy.h"
#include "KoShapeMoveStrategy.h"
#include "KoShapeRotateStrategy.h"
#include "KoShapeShearStrategy.h"
#include "KoShapeResizeStrategy.h"
#include "KoCreateShapeStrategy.h"
#include "KoCreateShapesTool.h"
#include "KoInteractionTool.h"
#include "KoCanvasBase.h"
#include "KoTool.h"
#include "KoShapeContainer.h"

#include <QUndoCommand>

#include <QMouseEvent>

void KoInteractionStrategy::cancelInteraction() {
    QUndoCommand *cmd = createCommand();
    if(cmd) {
        cmd->undo();
        delete cmd;
    }
}

KoInteractionStrategy::KoInteractionStrategy(KoTool *parent, KoCanvasBase *canvas)
: m_parent(parent)
, m_canvas(canvas)
{
}


void KoInteractionStrategy::applyGrid(QPointF &point) {
    // The 1e-10 here is a workaround for some weird division problem.
    // 360.00062366 / 2.83465058 gives 127 'exactly' when shown as a double,
    // but when casting into an int, we get 126. In fact it's 127 - 5.64e-15 !
    double gridX, gridY;
    m_canvas->gridSize(&gridX, &gridY);

    // This is a problem when calling applyGrid twice, we get 1 less than the time before.
    point.setX( static_cast<int>( point.x() / gridX + 1e-10 ) * gridX );
    point.setY( static_cast<int>( point.y() / gridY + 1e-10 ) * gridY );
}

QPointF KoInteractionStrategy::snapToGrid( const QPointF &point, Qt::KeyboardModifiers modifiers ) {
    if( ! m_canvas->snapToGrid() || (modifiers & Qt::ShiftModifier) )
        return point;
    QPointF p = point;
    applyGrid(p);
    return p;
}

