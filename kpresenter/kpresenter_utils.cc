/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kpresenter_utils.h"

#include <qpainter.h>
#include <qpointarray.h>
#include <qpoint.h>
#include <qcolor.h>
#include <qsize.h>
#include <kozoomhandler.h>
#include <koPoint.h>
/*========================== draw a figure =======================*/
void drawFigure( LineEnd figure, QPainter* painter, const KoPoint &coord, const QColor &color,int  _w, float angle, KoZoomHandler*_zoomHandler)
{
    painter->save();
    painter->setPen( Qt::NoPen );
    painter->setBrush( Qt::NoBrush );

    switch ( figure )
    {
    case L_SQUARE:
    {
        int _h = _w;
        if ( _h % 2 == 0 ) _h--;
	painter->translate( _zoomHandler->zoomItX(coord.x()), _zoomHandler->zoomItY( coord.y()) );
        painter->rotate( angle );
        painter->scale( 1, 1 );
        painter->fillRect( _zoomHandler->zoomItX(-3 - _w / 2),_zoomHandler->zoomItY( -3 - _h / 2),_zoomHandler->zoomItX( 6 + _w),_zoomHandler->zoomItY( 6 + _h), color );
    } break;
    case L_CIRCLE:
    {
	painter->translate( _zoomHandler->zoomItX(coord.x()), _zoomHandler->zoomItY(coord.y()) );
        painter->setBrush( color );
        painter->drawEllipse( _zoomHandler->zoomItX(-3 - _w / 2), _zoomHandler->zoomItY(-3 - _w / 2),_zoomHandler->zoomItX( 6 + _w), _zoomHandler->zoomItY(6 + _w) );
    } break;
    case L_ARROW:
    {
        QPoint p1( -5 - _w / 2, -3 - _w / 2 );
        QPoint p2( 5 + _w / 2, 0 );
        QPoint p3( -5 - _w / 2, 3 + _w / 2 );
        QPointArray pArray( 3 );
        pArray.setPoint( 0, _zoomHandler->zoomPoint(p1) );
        pArray.setPoint( 1, _zoomHandler->zoomPoint(p2) );
        pArray.setPoint( 2, _zoomHandler->zoomPoint(p3) );

	painter->translate( _zoomHandler->zoomItX(coord.x()),_zoomHandler->zoomItY( coord.y()) );
        painter->rotate( angle );
        painter->scale( 1, 1 );
        painter->setBrush( color );
        painter->drawPolygon( pArray );
    } break;
    case L_LINE_ARROW:
    {
        painter->translate( _zoomHandler->zoomItX(coord.x()),_zoomHandler->zoomItY( coord.y()) );
        painter->setPen( QPen(color , _zoomHandler->zoomItX( _w )) );
        painter->rotate( angle );
        painter->scale( 1, 1 );
        QPoint p1( _zoomHandler->zoomItX(-5 - _w / 2), _zoomHandler->zoomItY(-3 - _w / 2) );
        QPoint p2( _zoomHandler->zoomItX(5 + _w / 2), _zoomHandler->zoomItY(0) );
        QPoint p3( _zoomHandler->zoomItX(-5 - _w / 2), _zoomHandler->zoomItY(3 + _w / 2) );
        painter->drawLine( p2, p1);
        painter->drawLine( p2, p3);
    }break;
    case L_DIMENSION_LINE:
    {
        painter->translate( _zoomHandler->zoomItX(coord.x()),_zoomHandler->zoomItY( coord.y()) );
        painter->setPen( QPen(color , _zoomHandler->zoomItX( _w )) );
        painter->rotate( angle );
        painter->scale( 1, 1 );
        QPoint p1( _zoomHandler->zoomItX(0), _zoomHandler->zoomItY(-5 - _w / 2) );
        QPoint p2( _zoomHandler->zoomItX(0), _zoomHandler->zoomItY(5 + _w / 2 ) );
        painter->drawLine( p1, p2);
    }break;
    case L_DOUBLE_ARROW:
    {
	painter->translate( _zoomHandler->zoomItX(coord.x()),_zoomHandler->zoomItY( coord.y()) );
        painter->rotate( angle );
        painter->scale( 1, 1 );
        painter->setBrush( color );

        QPoint p1( -5 - _w / 2, -3 - _w / 2 );
        QPoint p2( 5 + _w / 2, 0 );
        QPoint p3( -5 - _w / 2, 3 + _w / 2 );

        QPoint p4( -15 - _w / 2, -3 - _w / 2 );
        QPoint p5( -5 + _w / 2, 0 );
        QPoint p6( -15 - _w / 2, 3 + _w / 2 );

        QPointArray pArray( 3 );
        pArray.setPoint( 0, _zoomHandler->zoomPoint(p1) );
        pArray.setPoint( 1, _zoomHandler->zoomPoint(p2) );
        pArray.setPoint( 2, _zoomHandler->zoomPoint(p3) );
        painter->drawPolygon( pArray );
        pArray.setPoint( 0, _zoomHandler->zoomPoint(p4) );
        pArray.setPoint( 1, _zoomHandler->zoomPoint(p5) );
        pArray.setPoint( 2, _zoomHandler->zoomPoint(p6) );
        painter->drawPolygon( pArray );

    }break;
    case L_DOUBLE_LINE_ARROW:
    {
        painter->translate( _zoomHandler->zoomItX(coord.x()),_zoomHandler->zoomItY( coord.y()) );
        painter->setPen( QPen(color , _zoomHandler->zoomItX( _w )) );
        painter->rotate( angle );
        painter->scale( 1, 1 );
        QPoint p1( _zoomHandler->zoomItX(-5 - _w / 2), _zoomHandler->zoomItY(-3 - _w / 2) );
        QPoint p2( _zoomHandler->zoomItX(5 + _w / 2), _zoomHandler->zoomItY(0) );
        QPoint p3( _zoomHandler->zoomItX(-5 - _w / 2), _zoomHandler->zoomItY(3 + _w / 2) );
        painter->drawLine( p2, p1);
        painter->drawLine( p2, p3);

        p1.setX( _zoomHandler->zoomItX(-15 - _w / 2));
        p2.setX( _zoomHandler->zoomItX(-5 + _w / 2));
        p3.setX( _zoomHandler->zoomItX(-15 - _w / 2));
        painter->drawLine( p2, p1);
        painter->drawLine( p2, p3);
    }break;
    default: break;
    }
    painter->restore();
}
//todo used kozoomhandled
/*================== get bounding with of figure =================*/
KoSize getBoundingSize( LineEnd figure, int _w, const KoZoomHandler*_zoomHandler )
{
    switch ( figure )
    {
    case L_SQUARE:
    {
        int _h = (int)_w;
        if ( _h % 2 == 0 ) _h--;
        return KoSize( _zoomHandler->zoomItX( 10 + _w), _zoomHandler->zoomItY( 10 + _h) );
    } break;
    case L_CIRCLE:
        return KoSize(  _zoomHandler->zoomItX(10 + _w), _zoomHandler->zoomItY(10 + _w) );
        break;
    case L_ARROW:
        return KoSize( _zoomHandler->zoomItX( 14 + _w),_zoomHandler->zoomItY( 14 + _w) );
        break;
    case L_LINE_ARROW:
        return KoSize( _zoomHandler->zoomItX( 14 + _w),_zoomHandler->zoomItY( 14 + _w) );
        break;
    case L_DIMENSION_LINE:
        return KoSize( _zoomHandler->zoomItX( 14 +_w),_zoomHandler->zoomItY( 14 + _w) );
        break;
    case L_DOUBLE_ARROW:
        return KoSize( _zoomHandler->zoomItX( 28 + _w),_zoomHandler->zoomItY( 14 + _w) );
        break;
    case L_DOUBLE_LINE_ARROW:
        return KoSize( _zoomHandler->zoomItX( 28 + _w),_zoomHandler->zoomItY( 14 + _w) );
        break;
    default: break;
    }

    return KoSize( 0, 0 );
}

QString lineEndBeginName( LineEnd type )
{
    switch(type)
    {
    case L_NORMAL:
        return QString("NORMAL");
    case L_ARROW:
        return QString("ARROW");
    case L_SQUARE:
        return QString("SQUARE");
    case L_CIRCLE:
        return QString("CIRCLE");
    case L_LINE_ARROW:
        return QString("LINE_ARROW");
    case L_DIMENSION_LINE:
        return QString("DIMENSION_LINE");
    case L_DOUBLE_ARROW:
        return QString("DOUBLE_ARROW");
    case L_DOUBLE_LINE_ARROW:
        return QString("DOUBLE_LINE_ARROW");
    }
    return QString::null;
}

LineEnd lineEndBeginFromString( const QString & type )
{
    if(type=="NORMAL")
        return L_NORMAL;
    else if(type=="ARROW")
        return L_ARROW;
    else if(type=="SQUARE")
        return L_SQUARE;
    else if(type=="CIRCLE")
        return L_CIRCLE;
    else if(type=="LINE_ARROW")
        return L_LINE_ARROW;
    else if (type=="DIMENSION_LINE")
        return L_DIMENSION_LINE;
    else if (type=="DOUBLE_ARROW")
        return L_DOUBLE_ARROW;
    else if (type=="DOUBLE_LINE_ARROW")
        return L_DOUBLE_LINE_ARROW;
    else
        kdDebug()<<"Error in LineEnd lineEndBeginFromString( const QString & name )\n";
    return L_NORMAL;
}

KoPointArray getCloseObject( KoPointArray points, bool close, bool objClosed )
{
    KoPointArray tmpPoints=points;
    if ( close )
    {
        tmpPoints.putPoints( points.count(), 1, points.at(0).x(), points.at(0).y());
    }
    else if ( objClosed)
    {
        tmpPoints.resize( points.count() - 1);
    }
    return tmpPoints;
}
