/***************************************************************************
                          cspecialfield.cpp  -  description
                             -------------------
    begin                : 07.06.2002
    copyright            : (C) 2002 by Alexander Dymo
    email                : cloudtemple@mksat.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <klocale.h>
#include "cspecialfield.h"

CanvasSpecialField::CanvasSpecialField(int x, int y, int width, int height, QCanvas * canvas):
	CanvasLabel(x, y, width, height, canvas)
{
    pair<QString, QStringList> propValues;
    
    propValues.first = i18n("CanvasSpecialField", "0");
    propValues.second << i18n("CanvasSpecialField", "Field type to display");
    propValues.second << "int_from_list"; 
    propValues.second << i18n("CanvasSpecialField", "0 - Date") 
	    << i18n("CanvasSpecialField", "1 - PageNumber");
    props["Type"] = propValues;
    propValues.second.clear();
    
    propValues.first = "11";
    propValues.second << i18n("CanvasSpecialField", "Date format");
    propValues.second << "int_from_list";
    propValues.second << i18n("CanvasSpecialField", "0 - m/d/y")
	    << i18n("CanvasSpecialField", "1 - m-d-y")
	    << i18n("CanvasSpecialField", "2 - mm/dd/y")
	    << i18n("CanvasSpecialField", "3 - mm-dd-y")
	    << i18n("CanvasSpecialField", "4 - m/d/yyyy")
	    << i18n("CanvasSpecialField", "5 - m-d-yyyy")
	    << i18n("CanvasSpecialField", "6 - mm/dd/yyyy")
	    << i18n("CanvasSpecialField", "7 - mm-dd-yyyy")
	    << i18n("CanvasSpecialField", "8 - yyyy/m/d")
	    << i18n("CanvasSpecialField", "9 - yyyy-m-d")
	    << i18n("CanvasSpecialField", "10 - dd.mm.yy")
	    << i18n("CanvasSpecialField", "11 - dd.mm.yyyy");
    props["DataType"] = propValues;
    propValues.second.clear();
}

void CanvasSpecialField::draw(QPainter &painter)
{
    props["Text"].first = "[" +  
			  QString(props["Type"].first.toInt()?"PageNo":"Date")
			  + "]";
    CanvasLabel::draw(painter);
}

QString CanvasSpecialField::getXml()
{
    return "\t\t<Special" + CanvasReportItem::getXml() + " />\n";
}
