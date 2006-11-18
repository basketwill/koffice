/* This file is part of the KDE project
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoEllipseShapeFactory.h"
#include "KoEllipseShape.h"
#include "KoLineBorder.h"

#include <klocale.h>

#include <QDebug>

KoEllipseShapeFactory::KoEllipseShapeFactory( QObject *parent )
: KoShapeFactory( parent, KoEllipseShapeId, i18n( "A ellipse shape" ) )
{
    setToolTip( i18n( "A ellipse" ) );
    setIcon("ellips");
}

KoShape * KoEllipseShapeFactory::createDefaultShape() 
{
    KoEllipseShape * ellipse = new KoEllipseShape();

    ellipse->setBorder( new KoLineBorder( 1.0 ) );
    ellipse->setShapeId( KoPathShapeId );
                 
    return ellipse;
}

KoShape * KoEllipseShapeFactory::createShape( const KoProperties * params ) const 
{
    Q_UNUSED(params);
    return new KoEllipseShape();
}

