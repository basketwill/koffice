/* This file is part of the KDE project
   Copyright (C) 2001, 2002, 2003 The Karbon Developers

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


#include <qregexp.h>
#include <qdom.h>

#include "vglobal.h"
#include "vpolygon.h"
#include "vtransformcmd.h"
#include <klocale.h>

VPolygon::VPolygon( VObject* parent, VState state ) 
	: VComposite( parent, state )
{
}

/*VPolygon::VPolygon( VObject* parent, const QString &points ) 
	: VComposite( parent ), m_points( points )
{
	init();
}*/

void
VPolygon::init()
{
	bool bFirst = true;

	QString points = m_points.simplifyWhiteSpace();
	points.replace( QRegExp( "," ), " " );
	points.replace( QRegExp( "\r" ), "" );
	points.replace( QRegExp( "\n" ), "" );
	QStringList pointList = QStringList::split( ' ', points );
	for( QStringList::Iterator it = pointList.begin(); it != pointList.end(); it++ )
	{
		if( bFirst )
		{
			moveTo( KoPoint( (*(it++)).toDouble(), (*it).toDouble() ) );
			bFirst = false;
		}
		else
			lineTo( KoPoint( (*(it++)).toDouble(), (*it).toDouble() ) );
	}
	close();
}

QString
VPolygon::name() const
{
	QString result = VObject::name();
	return !result.isEmpty() ? result : i18n( "Polygon" );
}

void
VPolygon::save( QDomElement& element ) const
{
	if( state() != deleted )
	{
		QDomElement me = element.ownerDocument().createElement( "POLYGON" );
		element.appendChild( me );

		VObject::save( me );

		me.setAttribute( "points", m_points );

		writeTransform( me );
	}
}

void
VPolygon::load( const QDomElement& element )
{
	setState( normal );

	QDomNodeList list = element.childNodes();
	for( uint i = 0; i < list.count(); ++i )
		if( list.item( i ).isElement() )
			VObject::load( list.item( i ).toElement() );

	m_points = element.attribute( "points" );

	init();

	QString trafo = element.attribute( "transform" );
	if( !trafo.isEmpty() )
		transform( trafo );
}

