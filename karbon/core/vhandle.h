/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
*/

#ifndef __VHANDLE_H__
#define __VHANDLE_H__

#include <qptrlist.h>
//#include "vobject.h"

class QPainter;
class VObject;

class VHandle
{
public:
	VHandle();
	~VHandle();

	void draw( QPainter& painter );

	void addObject( const VObject* object );
	void reset();

	// read-only access to objects:
	const QPtrList<VObject>& objects() const { return m_objects; }

private:
	// TODO : we must probably have 1 drawBox helper method, but where ?
	void drawBox( QPainter& painter, double x, double y, uint handleSize = 3 );

private:
	QPtrList<VObject> m_objects;
};

#endif
