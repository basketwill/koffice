/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
   Copyright (C) 2002, The Karbon Developers
*/

#include <klocale.h>

#include "vgroup.h"
#include "vgroupcmd.h"


VGroupCmd::VGroupCmd( VDocument *doc )
	: VCommand( doc, i18n( "Group Objects" ) )
{
	m_objects = m_doc->selection();
	m_group = 0L;
}

void
VGroupCmd::execute()
{
	m_group = new VGroup();

	VObjectListIterator itr( m_objects );
	for ( ; itr.current() ; ++itr )
	{
		// TODO : remove from corresponding VLayer
		m_doc->activeLayer()->removeRef( itr.current() );
		m_group->insertObject( itr.current() );
	}
	m_doc->insertObject( m_group );
	m_doc->selectObject( *m_group, true );
}

void
VGroupCmd::unexecute()
{
	m_doc->deselectAllObjects();
	VObjectListIterator itr( m_group->objects() );
	for ( ; itr.current() ; ++itr )
	{
		// TODO : remove from corresponding VLayer
		//m_part->insertObject( itr.current() );
		m_doc->selectObject( *( itr.current() ) );
	}
	// TODO : remove from corresponding VLayer
	//static_cast<VLayer *>( m_group->parent() )->removeRef( m_group );
	m_group->ungroup();
	delete m_group;
	m_group = 0L;
}

