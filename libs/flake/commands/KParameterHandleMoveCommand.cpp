/* This file is part of the KDE project
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006,2007 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#include "KParameterHandleMoveCommand_p.h"
#include "KParameterShape.h"
#include <klocale.h>

KParameterHandleMoveCommand::KParameterHandleMoveCommand(KParameterShape *shape, int handleId, const QPointF &startPoint, const QPointF &endPoint, Qt::KeyboardModifiers keyModifiers, QUndoCommand *parent)
        : QUndoCommand(parent)
        , m_shape(shape)
        , m_handleId(handleId)
        , m_startPoint(startPoint)
        , m_endPoint(endPoint)
        , m_keyModifiers(keyModifiers)
{
    setText(i18n("Change Parameter"));
}

KParameterHandleMoveCommand::~KParameterHandleMoveCommand()
{
}

/// redo the command
void KParameterHandleMoveCommand::redo()
{
    QUndoCommand::redo();
    m_shape->update();
    m_shape->moveHandle(m_handleId, m_endPoint, m_keyModifiers);
    m_shape->update();
}

/// revert the actions done in redo
void KParameterHandleMoveCommand::undo()
{
    QUndoCommand::undo();
    m_shape->update();
    m_shape->moveHandle(m_handleId, m_startPoint);
    m_shape->update();
}

