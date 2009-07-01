/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
                      Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
                 2006 Martin Pfeiffer <hubipete@gmx.net>

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

#include "RowElement.h"
#include "FormulaCursor.h"
#include <KoXmlWriter.h>
#include <KoXmlReader.h>
#include <QPainter>

#include <kdebug.h>

RowElement::RowElement( BasicElement* parent ) : BasicElement( parent )
{}

RowElement::~RowElement()
{
    qDeleteAll( m_childElements );
}

void RowElement::paint( QPainter& painter, AttributeManager* )
{ /* RowElement has no visual representance so paint nothing */ }

void RowElement::layout( const AttributeManager* am )
{
    Q_UNUSED( am )          // there are no attributes that can be processed here

    if( m_childElements.isEmpty() )  // do not do anything if there are no children
        return;

    QPointF origin;
    double width = 0.0;
    double topToBaseline = 0.0;
    double baselineToBottom = 0.0;
    foreach( BasicElement* child, m_childElements ) // iterate through the children and
        topToBaseline = qMax( topToBaseline, child->baseLine() );  // find max baseline

    foreach( BasicElement* child, m_childElements )  // iterate through the children
    {
        child->setOrigin( QPointF( width, topToBaseline - child->baseLine() ) );
        baselineToBottom = qMax( baselineToBottom, child->height()-child->baseLine() );
        width += child->width();       // add their width
    }

    setWidth( width );
    setHeight( topToBaseline + baselineToBottom );
    setBaseLine( topToBaseline );
    setChildrenBoundingRect(QRectF(0,0, width, height()));
}

void RowElement::stretch()
{
    //The elements can grow vertically, so make sure we reposition their vertical 
    //origin appropriately
    foreach( BasicElement* tmpElement, childElements() ) {
        tmpElement->stretch();
        //Set the origin.  Note that we ignore the baseline and center the object
        //vertically
        //I think we need to FIXME for symmetric situations or something?
        tmpElement->setOrigin( QPointF(tmpElement->origin().x(), childrenBoundingRect().y() + (childrenBoundingRect().height() - tmpElement->height())/2 ));
    }
}

int RowElement::length() const
{
    return m_childElements.count();
}

const QList<BasicElement*> RowElement::childElements()
{
    return m_childElements;
}

void RowElement::insertChild( FormulaCursor* cursor, BasicElement* child )
{
    if( cursor->currentElement() == this )
        m_childElements.insert( cursor->position(), child );
    kDebug() << "inserting child in Row at " <<cursor->position();
    // else
    //     TODO make some error
}

void RowElement::removeChild( FormulaCursor* cursor, BasicElement* child )
{
    Q_UNUSED( cursor )
    m_childElements.removeOne( child );
}

BasicElement* RowElement::acceptCursor( const FormulaCursor* cursor )
{
    return this;
}

bool RowElement::moveCursor(FormulaCursor* cursor) {
    if ( (cursor->direction()==MoveUp) ||
	 (cursor->direction()==MoveDown)) {
	//the cursor can't be moved vertically
	//TODO: check what happens with linebreaks in <mspace> elements
	return false;
    }
    if ( (cursor->isHome() && cursor->direction()==MoveLeft) ||
	 (cursor->isEnd() && cursor->direction()==MoveRight) ) {
	return BasicElement::moveCursor(cursor);
    }
    switch(cursor->direction()) {
	case MoveLeft:
	    cursor->setCurrentElement(m_childElements[cursor->position()-1]);
	    cursor->moveEnd();
	    break;
	case MoveRight:
	    cursor->setCurrentElement(m_childElements[cursor->position()]);
	    cursor->moveHome();
	    break;
    }
    return true;
}

QLineF RowElement::cursorLine(const FormulaCursor* cursor) {
    QPointF top=absoluteBoundingRect().topLeft();
    if( childElements().isEmpty() ) {
        // center cursor in elements that have no children
        top += QPointF( width()/2, 0 );
    } else { 
	if ( cursor->isEnd()) {
	    top += QPointF(width(),0.0);
	} else {
	    top += QPointF( childElements()[ cursor->position() ]->boundingRect().left(), 0.0 );
	}
    }
    QPointF bottom = top + QPointF( 0.0, height() );
    return QLineF(top, bottom);
}

bool RowElement::setCursorTo(FormulaCursor* cursor, QPointF point)
{
    if (point.x()<0) {
	cursor->setCurrentElement(this);
	cursor->setPosition(0);
	return true;
    }
    QList<BasicElement*>::const_iterator tmp;
    for (tmp=childElements().begin(); tmp!=childElements().end(); tmp++) {
	//Find the child element the point is in
	if ((*tmp)->boundingRect().right()>=point.x()) {
	    break;
	}
    }
    //check if the point is behind all child elements
    if (tmp==childElements().end()) {
	cursor->setCurrentElement(this);
	cursor->setPosition(length());
	return true;
    }
    point-=(*tmp)->origin();
    return (*tmp)->setCursorTo(cursor,point);
}


ElementType RowElement::elementType() const
{
    return Row;
}

bool RowElement::readMathMLContent( const KoXmlElement& parent )
{
    BasicElement* tmpElement = 0;
    KoXmlElement tmp;
    forEachElement( tmp, parent )
    {
        tmpElement = ElementFactory::createElement( tmp.tagName(), this );
        Q_ASSERT( tmpElement );
        m_childElements << tmpElement;
        if( !tmpElement->readMathML( tmp ) )
            return false;
    }
    return true;
}

int RowElement::positionOfChild(BasicElement* child) const {
    return m_childElements.indexOf(child);
}

void RowElement::writeMathMLContent( KoXmlWriter* writer ) const
{
    foreach( BasicElement* tmp, m_childElements )
        tmp->writeMathML( writer );
}

