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

#include "karbon_drag.h"

#include <qcstring.h>
#include <qdom.h>
#include <qtextstream.h>

#include "vdocument.h"

KarbonDrag::KarbonDrag(QWidget* dragSource, const char* name)
 : QDragObject(dragSource, name)
{
	m_encodeFormats[0] = "application/vnd.kde.karbon";
	m_decodeFormats[0] = "application/vnd.kde.karbon";
}

const char* KarbonDrag::format(int i) const
{
	if(i < NumEncodeFmts) {
		return m_encodeFormats[i];
	}

	return 0L;
}

QByteArray KarbonDrag::encodedData(const char* mimetype) const
{
	QCString result;

	if(m_encodeFormats[0] == mimetype) {
		VObjectListIterator itr( m_objects );
		// build a xml fragment containing the selection as karbon xml
		QDomDocument doc( "clip" );
		QDomElement elem = doc.createElement( "clip" );
		QTextStream ts( result, IO_WriteOnly );

		for( ; itr.current() ; ++itr )
			itr.current()->save( elem );

		ts << elem;
	}

	return result;
}

bool KarbonDrag::canDecode(QMimeSource* e)
{
	for(int i = 0; i < NumDecodeFmts; i++) {
		if(e->provides(m_decodeFormats[i])) {
			return true;
		}
	}

	return false;
}

bool KarbonDrag::decode(QMimeSource* e, VObjectList& sl, VDocument& vdoc)
{
	if(e->provides(m_decodeFormats[0])) {
		QDomDocument doc( "clip" );
		QByteArray data = e->encodedData(m_decodeFormats[0]);
		doc.setContent( QCString( data, data.size()+1 ) );
		QDomElement clip = doc.documentElement();
		// Try to parse the clipboard data
		if( clip.tagName() == "clip" ) {
			VObjectList selection;
			VGroup grp( &vdoc );
			grp.load( clip );
			VObjectListIterator itr( grp.objects() );
			for( ; itr.current() ; ++itr ) {
				sl.append(itr.current()->clone());
			}

			return true;
		}
	}

	return false;
}

void KarbonDrag::setObjectList(VObjectList l)
{
	VObjectListIterator itr( l );
	m_objects.clear();

	for( ; itr.current() ; ++itr ) {
		m_objects.append(itr.current()->clone());
	}
}

#include "karbon_drag.moc"
