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

#ifndef __VDRAWSELECTION_H__
#define __VDRAWSELECTION_H__

#include "vgroup.h"
#include "vvisitor.h"

/**
 *  Helper class to draw the outline of a composite path, including (?)
 *  optionally its bezier helper lines, depending on the state.
 */
class VDrawSelection : public VVisitor
{
public:
	VDrawSelection( const VObjectList& selection, VPainter *painter ) : m_selection( selection ), m_painter( painter ) {}

	virtual void visitVComposite( VComposite& composite );

private:
	VObjectList		m_selection;
	VPainter		*m_painter;
};

#endif

