/*
** A program to convert the XML rendered by KSpread into LATEX.
**
** Copyright (C) 2003 Robert JACOLIN
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** To receive a copy of the GNU Library General Public License, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
**
*/

#include <kdebug.h>		/* for kdDebug stream */

#include "row.h"

/*******************************************/
/* Constructor                             */
/*******************************************/
Row::Row(): Format()
{
	setRow(0);
}

/*******************************************/
/* Destructor                              */
/*******************************************/
Row::~Row()
{
}

void Row::analyse(const QDomNode balise)
{
	_row = getAttr(balise, "row").toLong();
	_height = getAttr(balise, "height").toDouble();
	Format::analyse(getChild(balise, "format"));
}

/*******************************************/
/* generate                                */
/*******************************************/
void Row::generate(QTextStream& out)
{
	//generateTopBorder(out);
	if(getBrushStyle() >= 1)
	{
		out << "\\rowcolor";
		generateColor(out);
	}
	//generateBottomBorder(out);
		
	//out << "m{" << getHeight() << "pt}";
	
}


