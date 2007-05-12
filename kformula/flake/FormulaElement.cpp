/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
   Copyright (C) 2006-2007 Martin Pfeiffer <hubipete@gmx.net>
   Copyright (C) 2006-2007 Alfredo Beaumont Sainz <alfredo.beaumont@gmail.com>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "FormulaElement.h"
#include "FormulaCursor.h"
#include "ElementFactory.h"
#include <KoXmlWriter.h>

namespace FormulaShape {

FormulaElement::FormulaElement() : BasicElement( 0 )
{
}

void FormulaElement::moveLeft( FormulaCursor* cursor, BasicElement* from )
{
    Q_UNUSED( cursor )
    Q_UNUSED( from )
}

void FormulaElement::moveRight( FormulaCursor* cursor, BasicElement* from )
{
    Q_UNUSED( cursor )
    Q_UNUSED( from )
}

void FormulaElement::moveUp( FormulaCursor* cursor, BasicElement* from )
{
    Q_UNUSED( cursor )
    Q_UNUSED( from )
}

void FormulaElement::moveDown( FormulaCursor* cursor, BasicElement* from )
{
    Q_UNUSED( cursor )
    Q_UNUSED( from )
}

bool FormulaElement::readMathMLContent( const KoXmlElement& parent )
{
    BasicElement* tmpElement = 0;
    KoXmlElement tmp;
    forEachElement( tmp, parent )
    {
        tmpElement = ElementFactory::createElement( tmp.localName(), this );
        m_childElements << tmpElement;
        tmpElement->readMathML( tmp );
    }

    return true;
}

void FormulaElement::writeMathMLContent( KoXmlWriter* writer ) const
{
    foreach( BasicElement* tmpChild, m_childElements )       // just write all
        tmpChild->writeMathML( writer );                   // children elements
}
ElementType FormulaElement::elementType() const
{
    return Formula;
}

} // namespace FormulaShape
