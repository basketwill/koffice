/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>

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

#ifndef __CONTEXTSTYLE_H
#define __CONTEXTSTYLE_H

//KDE Include
//#include <kconfig.h>

//Qt Include
#include <qcolor.h>
#include <qfont.h>

//Formula include


/**
 * Contains all the style information for the formela. The idea
 * is to change the values here (user configurable) and have
 * the elements paint themselves with this information.
 */
class ContextStyle
{
public:  
    enum Alignment {left, center, right};

    /**
     * Build a default context style
     */
    ContextStyle();

    /**
     * Build a context style reading settings from config
     */
    //ContextStyle(KConfig *config);

    
    QColor getDefaultColor()  { return defaultColor; }
    QColor getNumberColor()   { return numberColor; }
    QColor getOperatorColor() { return operatorColor; }
    QColor getErrorColor()    { return errorColor; }

    QFont getDefaultFont()    { return defaultFont; }
    QFont getNameFont()       { return nameFont; }
    QFont getNumberFont()     { return numberFont; }
    QFont getOperatorFont()   { return operatorFont; }
    QFont getSymbolFont()     { return symbolFont; }

    int getDistance() { return 5; }

    Alignment getMatrixAlignment() { return center; }
    
private:
    
    QFont defaultFont;
    QFont nameFont;
    QFont numberFont;
    QFont operatorFont;
    QFont symbolFont;
    
    QColor defaultColor;
    QColor numberColor;
    QColor operatorColor;
    QColor errorColor;
};

#endif // __CONTEXTSTYLE_H
