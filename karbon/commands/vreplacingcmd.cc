/* This file is part of the KDE project
   Copyright (C) 2002, The Karbon Developers

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


#include "vreplacingcmd.h"
#include "vselection.h"


VReplacingCmd::VReplacingCmd( VDocument* doc, const QString& name )
	: VCommand( doc, name )
{
	// Set members.
	m_oldObjects = document()->selection()->clone();
	m_newObjects = 0L;
}

VReplacingCmd::~VReplacingCmd()
{
	delete( m_oldObjects );
	delete( m_newObjects );
}

void
VReplacingCmd::execute()
{
	// Create new shapes if they don't exist yet.
	if( !m_newObjects )
	{
		m_newObjects = new VSelection();

		// Pointer to temporary object.
		VObject* newObject;

		VObjectListIterator itr( m_oldObjects->objects() );

		for( ; itr.current(); ++itr )
		{
			// Reset success.
			setSuccess( false );

			// Clone object and visit the clone.
			newObject = itr.current()->clone();
			visit( *newObject );

			// Success.
			if( success() )
			{
				// Insert new shape right before old shape.
				itr.current()->parent()->insertInfrontOf(
					newObject, itr.current() );

				// Add new shape to list of new objects.
				m_newObjects->append( newObject );
			}
			// No success.
			else
			{
				// Don't consider this object in the future anymore.
				m_oldObjects->take( *itr.current() );

				// Delete temporary object.
				delete( newObject );
			}
		}
	}

	// Nothing to do.
	if( m_newObjects->objects().count() == 0 )
		return;

	VObjectListIterator itr( m_oldObjects->objects() );

	// Hide old objects.
	for( ; itr.current(); ++itr )
	{
		document()->selection()->take( *itr.current() );
		itr.current()->setState( VObject::deleted );
	}

	// Show new objects.
	for( itr = m_newObjects->objects(); itr.current(); ++itr )
	{
		itr.current()->setState( VObject::normal );
		document()->selection()->append( itr.current() );
	}
}

void
VReplacingCmd::unexecute()
{
	// Nothing to do.
	if( m_newObjects->objects().count() == 0 )
		return;

	VObjectListIterator itr( m_oldObjects->objects() );

	// Show old objects.
	for( ; itr.current(); ++itr )
	{
		itr.current()->setState( VObject::normal );
		document()->selection()->append( itr.current() );
	}

	// Hide new objects.
	for( itr = m_newObjects->objects(); itr.current(); ++itr )
	{
		document()->selection()->take( *itr.current() );
		itr.current()->setState( VObject::deleted );
	}
}
