/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
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

#ifndef __VSTARTOOL_H__
#define __VSTARTOOL_H__

#include <qgroupbox.h>

#include <klocale.h>

#include "vshapetool.h"


class KDoubleNumInput;
class KIntSpinBox;
class KarbonPart;

class VStarOptionsWidget : public QGroupBox
{
	Q_OBJECT

public:
	VStarOptionsWidget( KarbonPart*part, QWidget* parent = 0L, const char* name = 0L );

	double innerR() const;
	double outerR() const;
	uint edges() const;
	void setInnerR( double value );
	void setOuterR( double value );
	void setEdges( uint value );
	void refreshUnit ();
private:
	KDoubleNumInput* m_innerR;
	KDoubleNumInput* m_outerR;
	KIntSpinBox* m_edges;
	KarbonPart*m_part;
	QLabel *m_innerRLabel;
	QLabel *m_outerRLabel;
};


class VStarTool : public VShapeTool
{
public:
	VStarTool( KarbonView* view );
	virtual ~VStarTool();

	virtual QWidget* optionsWidget() { return m_optionsWidget; }
	virtual QString name() { return i18n( "Star tool" ); }
	
	virtual VComposite* shape( bool interactive = false ) const;

	void refreshUnit();
	
private:
	VStarOptionsWidget* m_optionsWidget;
};

#endif

