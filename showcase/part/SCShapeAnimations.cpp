/* This file is part of the KDE project
 * Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2010 Benjamin Port <port.benjamin@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or ( at your option) any later version.
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
#include "SCShapeAnimations.h"
#include <QList>
#include <QSet>
#include <kdebug.h>
#include "KShape.h"

SCShapeAnimations::SCShapeAnimations()
{
}

SCShapeAnimations::~SCShapeAnimations()
{
}

void SCShapeAnimations::init(const QList<SCAnimationStep *> animations)
{
    m_shapeAnimations = animations;
}

void SCShapeAnimations::add(SCShapeAnimation * animation)
{
    Q_UNUSED(animation);
}

void SCShapeAnimations::remove(SCShapeAnimation * animation)
{
    Q_UNUSED(animation);
}

QList<SCAnimationStep *> SCShapeAnimations::steps() const
{
    return m_shapeAnimations;
}
