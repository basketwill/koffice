// -*- Mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4; -*-
/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kprectobject.h"
#include "kpgradient.h"
#include "KPRectObjectIface.h"

#include <kdebug.h>
#include <qregion.h>
#include <qbitmap.h>
#include <qdom.h>
#include <qpainter.h>
#include <kozoomhandler.h>
#include <koUnit.h>
#include <kooasiscontext.h>

KPRectObject::KPRectObject()
    : KP2DObject()
{
    xRnd = 0;
    yRnd = 0;
}

DCOPObject* KPRectObject::dcopObject()
{
    if ( !dcop )
        dcop = new KPRectObjectIface( this );
    return dcop;
}

KPRectObject::KPRectObject( const QPen &_pen, const QBrush &_brush, FillType _fillType,
                            const QColor &_gColor1, const QColor &_gColor2,
                            BCType _gType, int _xRnd, int _yRnd,
                            bool _unbalanced, int _xfactor, int _yfactor)
    : KP2DObject( _pen, _brush, _fillType, _gColor1, _gColor2, _gType,
                  _unbalanced, _xfactor, _yfactor )
{
    xRnd = _xRnd;
    yRnd = _yRnd;
}

KPRectObject &KPRectObject::operator=( const KPRectObject & )
{
    return *this;
}

QDomDocumentFragment KPRectObject::save( QDomDocument& doc, double offset )
{
    QDomDocumentFragment fragment=KP2DObject::save(doc, offset);
    if (xRnd!=0 || yRnd!=0) {
        QDomElement elem=doc.createElement("RNDS");
        elem.setAttribute("x", xRnd);
        elem.setAttribute("y", yRnd);
        fragment.appendChild(elem);
    }
    return fragment;
}

bool KPRectObject::saveOasis( KoXmlWriter &xmlWriter, KoSavingContext& context, int indexObj  ) const
{
    xmlWriter.startElement( "draw:rect" );
    xmlWriter.addAttribute( "draw:style-name", KP2DObject::saveOasisBackgroundStyle( xmlWriter, context.mainStyles(), indexObj ) );

    if( !objectName.isEmpty())
        xmlWriter.addAttribute( "draw:name", objectName );
    xmlWriter.endElement();
    return true;
}


void KPRectObject::loadOasis(const QDomElement &element, KoOasisContext&context, KPRLoadingInfo *info)
{
    KP2DObject::loadOasis(element, context, info);
    if ( element.hasAttribute( "draw:corner-radius" ) )
    {
        //todo FIXME, conversion is not good, oo give radius and kpresenter give xRnd and yRnd 0->99
        int radius = static_cast<int>( KoUnit::parseValue( element.attribute( "draw:corner-radius" ) ) );
        xRnd = radius;
        yRnd = radius;
        kdDebug()<<" KPRectObject : radius xRnd :"<<xRnd <<" yRnd :"<<yRnd<<endl;
    }
}

double KPRectObject::load(const QDomElement &element)
{
    double offset=KP2DObject::load(element);
    QDomElement e=element.namedItem("RNDS").toElement();
    if(!e.isNull()) {
        int tmp=0;
        if(e.hasAttribute("x"))
            tmp=e.attribute("x").toInt();
        xRnd=tmp;
        tmp=0;
        if(e.hasAttribute("y"))
            tmp=e.attribute("y").toInt();
        yRnd=tmp;
    }
    return offset;
}

QPointArray KPRectObject::boundingRegion( int x, int y, int w, int h, int _xRnd, int _yRnd) const
{
    w--;
    h--;
    int rxx = (_xRnd==0)?1: (w*_xRnd/200);
    int ryy = (_yRnd==0)?1: (h*_yRnd/200);
    // were there overflows?
    if ( rxx < 0 )
        rxx = w/200*_xRnd;
    if ( ryy < 0 )
        ryy = h/200*_yRnd;
    int rxx2 = 2*rxx;
    int ryy2 = 2*ryy;
    QPointArray a[4];
    a[0].makeArc( x, y, rxx2, ryy2, 1440, 1440); // ..., 1*16*90, 16*90);
    a[1].makeArc( x, y+h-ryy2, rxx2, ryy2, 2880, 1440); // ..., 2*16*90, 16*90);
    a[2].makeArc( x+w-rxx2, y+h-ryy2, rxx2, ryy2, 4320, 1440); // ..., 3*16*90, 16*90);
    a[3].makeArc( x+w-rxx2, y, rxx2, ryy2, 0, 1440); // ..., 0*16*90, 16*90);
    QPointArray aa;
    aa.resize( a[0].size() + a[1].size() + a[2].size() + a[3].size() );
    uint j = 0;
    for ( int k=0; k<4; k++ ) {
        for ( uint i=0; i<a[k].size(); i++ ) {
            aa.setPoint( j, a[k].point(i) );
            j++;
        }
    }
    return aa;
}

void KPRectObject::paint( QPainter* _painter, KoZoomHandler*_zoomHandler,
                          int /* pageNum */, bool drawingShadow, bool drawContour )
{
    int ow = _zoomHandler->zoomItX( ext.width() );
    int oh = _zoomHandler->zoomItY( ext.height() );

    if ( drawContour ) {
        QPen pen3( Qt::black, 1, Qt::DotLine );
        _painter->setPen( pen3 );
        _painter->setRasterOp( Qt::NotXorROP );

        _painter->drawRoundRect( 0, 0, ow, oh,
                                 _zoomHandler->zoomItX( xRnd ),_zoomHandler->zoomItY( yRnd) );
        return;
    }

    QPen pen2(pen);
    pen2.setWidth(_zoomHandler->zoomItX(pen.width()));
    int pw = ( pen2.style() == Qt::NoPen ) ? 1 : pen2.width();
    _painter->setPen( pen2 );

    if ( drawingShadow || fillType == FT_BRUSH || !gradient ) { //plain fill
        _painter->setPen( pen2 );
        _painter->setBrush( brush );
    }
    else { //gradient
        QSize size( _zoomHandler->zoomSize( ext ) );
        gradient->setSize( size );

        QPixmap pix( ow - pw + 1, oh - pw + 1);

        QPointArray arr = boundingRegion( 0, 0, ow - pw + 1, oh - pw + 1,
                                          _zoomHandler->zoomItX(xRnd),
                                          _zoomHandler->zoomItY(yRnd) );
        QRegion clipregion(arr);

        pix.resize ( ow, oh );
        pix.fill( Qt::white );

        QPainter p;
        p.begin( &pix );
        p.setClipRegion( clipregion );
        p.drawPixmap( 0, 0, gradient->pixmap() );
        p.end();

        pix.setMask( pix.createHeuristicMask() );
        _painter->drawPixmap( pw / 2, pw / 2, pix, 0, 0, ow - pw + 1, oh - pw + 1 );

        _painter->setBrush( Qt::NoBrush );
    }

    _painter->drawRoundRect( pw / 2, pw / 2, ow - pw + 1, oh - pw + 1,
                             _zoomHandler->zoomItX( xRnd ),_zoomHandler->zoomItY( yRnd) );
}
