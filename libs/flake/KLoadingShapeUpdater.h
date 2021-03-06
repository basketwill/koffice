/* This file is part of the KDE project
   Copyright (C) 2009 Thorsten Zachmann <zachmann@kde.org>

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

#ifndef KLOADINGSHAPEUPDATER_H
#define KLOADINGSHAPEUPDATER_H

#include "flake_export.h"

class KShape;

/**
 * Reimplement this class when you depend on a shape during loading that is
 * not yet loaded.
 *
 * As soon as the shape you depend on is loaded the method update is called.
 * Then you can setup the data you need.
 *
 * @see KShapeConnection
 * @see KShapeLoadingContext::updateShape
 */
class FLAKE_EXPORT KLoadingShapeUpdater
{
public:
    KLoadingShapeUpdater();
    virtual ~KLoadingShapeUpdater();

    /**
     * This function is called as soon as shape is loaded.
     *
     * @param shape The shape that just got loaded.
     */
    virtual void update(KShape *shape) = 0;
};

#endif /* KOLOADINGSHAPEUPDATER_H */
