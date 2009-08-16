/* This file is part of thShapee KDE project
   Copyright (C) 2006 Martin Pfeiffer <hubipete@gmx.net>
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

#include "UnderOverElement.h"
#include "FormulaCursor.h"
#include "AttributeManager.h"
#include <KoXmlReader.h>
#include <kdebug.h>
#include <QPainter>

UnderOverElement::UnderOverElement( BasicElement* parent, ElementType elementType ) : FixedElement( parent )
{
    m_baseElement = new RowElement( this );
    m_underElement = new RowElement( this );
    m_overElement = new RowElement( this );
    m_elementType = elementType;
}

UnderOverElement::~UnderOverElement()
{
    delete m_baseElement;
    delete m_underElement;
    delete m_overElement;
}

const QList<BasicElement*> UnderOverElement::childElements() const
{
    QList<BasicElement*> tmp;
    tmp << m_baseElement;
//     if(m_elementType != Over)
    tmp << m_overElement;
//     if(m_elementType != Under)
    tmp << m_underElement;
    return tmp;
}

void UnderOverElement::paint( QPainter& painter, AttributeManager* am)
{
    /*do nothing as UnderOverElement has no visual representance*/
    Q_UNUSED(painter)
    Q_UNUSED(am)
}

void UnderOverElement::layout( const AttributeManager* am )
{
//     double thinSpace   = am->layoutSpacing( this );
//     double accent      = m_elementType != Under && am->boolOf( "accent", this );     //Whether to add a space above
//     double accentUnder = m_elementType != Over && am->boolOf( "accentunder", this );//Whether to add a space below

    // Set whether to stretch the element.  Set it to true if it doesn't exist to make it easy to check if any are non-stretchy
    bool underStretchy = m_elementType == Over || am->boolOf( "stretchy", m_underElement );
    bool overStretchy  = m_elementType == Under || am->boolOf( "stretchy", m_overElement );
    bool baseStretchy  = (underStretchy && overStretchy) || am->boolOf( "stretchy", m_baseElement );  //For sanity, make sure at least one is not stretchy

    double largestWidth = 0;
    if(!baseStretchy)
        largestWidth = m_baseElement->width();

    if(m_elementType != Over && !underStretchy)
        largestWidth = qMax( m_underElement->width(), largestWidth );
    if(m_elementType != Under && !overStretchy)
        largestWidth = qMax( m_overElement->width(), largestWidth );

    QPointF origin(0.0,0.0);
    if(m_elementType != Under) {
        origin.setX(( largestWidth - m_overElement->width() ) / 2.0 ) ;
        m_overElement->setOrigin( origin );
        origin.setY( m_overElement->height() );
    }

    origin.setX( ( largestWidth - m_baseElement->width() ) / 2.0 );
    m_baseElement->setOrigin( origin );
    setBaseLine( origin.y() + m_baseElement->baseLine() );

    if(m_elementType != Over) {
        origin.setX( ( largestWidth - m_underElement->width())/2.0 );
	/* Try to be smart about where to place the under */
//	if(m_baseElement->baseLine() + 1.5*thinSpace > m_baseElement->height())
//          origin.setY( origin.y() + m_baseElement->baseLine() );
//	else 
          origin.setY( origin.y() + m_baseElement->height()); 
        m_underElement->setOrigin( origin );
        setHeight( origin.y() + m_underElement->height() );
    } else {
        setHeight( origin.y() + m_baseElement->height() );
    }

    setWidth( largestWidth );
}

QString UnderOverElement::attributesDefaultValue( const QString& attribute ) const
{
    Q_UNUSED( attribute )
/*    if( m_overElement->elementType() == Operator )
    else if( m_underElement->elementType() == Operator )*/
    return "false";  // the default for accent and
}

bool UnderOverElement::readMathMLContent( const KoXmlElement& parent )
{
    BasicElement* tmpElement = 0;
    KoXmlElement tmp;
    bool baseElement = true;
    bool underElement = true;
    bool overElement = true;
    forEachElement( tmp, parent ) { 
        tmpElement = ElementFactory::createElement( tmp.tagName(), this );
        if( !tmpElement->readMathML( tmp ) ) {
            return false;
        }

        if( baseElement ) {
            delete m_baseElement; 
            m_baseElement = tmpElement;
            baseElement = false;
        } else if( underElement && m_elementType != Over ) {
            delete m_underElement;
            m_underElement = tmpElement;
            underElement = false;
        } else if( overElement ) {
            delete m_overElement;
            m_overElement = tmpElement;
            overElement = false;
        } else {
            return false;
        }
    }
    return true;
} 

void UnderOverElement::writeMathMLContent( KoXmlWriter* writer ) const
{
    m_baseElement->writeMathML( writer );   // Just save the children in
    if(m_elementType != Over)
        m_underElement->writeMathML( writer );  // the right order
    if(m_elementType != Under)
        m_overElement->writeMathML( writer );
}

ElementType UnderOverElement::elementType() const
{
    return m_elementType;
}

bool UnderOverElement::moveCursor ( FormulaCursor& newcursor, FormulaCursor& oldcursor )
{
    int childpos=newcursor.position()/2;
    switch (childpos) {
        case 1:
            return moveVertSituation(newcursor,oldcursor,1,0);
            break;
        case 0:
            if (newcursor.direction()==MoveDown) {
                return moveVertSituation(newcursor,oldcursor,0,2);
            } else if (newcursor.direction()==MoveUp) {
                return moveVertSituation(newcursor,oldcursor,1,0);
            } else {
                return moveVertSituation(newcursor,oldcursor,0,1);
            }
            break;
        case 2:
            return moveVertSituation(newcursor,oldcursor,0,2);
        default:
            return false;
    }
}

int UnderOverElement::length() const
{
    return 5;
}

bool UnderOverElement::setCursorTo ( FormulaCursor& cursor, QPointF point )
{
    if (cursor.isSelecting()) {
        return false;
    }
    if (m_underElement->boundingRect().contains(point)) {
        return m_underElement->setCursorTo(cursor, point-m_underElement->origin());
    } else if (m_overElement->boundingRect().contains(point)) {
        return m_overElement->setCursorTo(cursor, point-m_overElement->origin());
    } else {
        return m_baseElement->setCursorTo(cursor, point-m_baseElement->origin());
    }
}
