/* This file is part of the KDE project
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006 Casper Boemann Rasmussen <cbr@boemann.dk>
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

#ifndef KORECTANGLESHAPE_H
#define KORECTANGLESHAPE_H

#include <KoShape.h>
#include <KoShapeFactory.h>

#include <koffice_export.h>

class QPainter;

/**
 * Simple Rectangle shape.
 */
class FLAKE_EXPORT KoRectangleShape : public KoShape
{
public:
    KoRectangleShape();
    void paint(QPainter &painter, KoViewConverter &converter);
};

class KoRectangleShapeFactory : public KoShapeFactory
{
public:
    // XXX: Add a nice pixmap
    KoRectangleShapeFactory() 
    {
	setName("RectangleShape");
	setDescription("A simple square shape");
	setToolTip("A simple square shape");
    }

    KoShape * createDefaultShape()
    {
	return new KoRectangleShape();
    }

    KoShape * createShape(KoShapeParameters * params) const
    {
	Q_UNUSED(params);
	return new KoRectangleShape();
    }

    KoShape * createShapeFromTemplate(KoShapeTemplate * shapeTemplate) const
    {
	Q_UNUSED(shapeTemplate);
	return new KoRectangleShape();
    }
};


#endif

