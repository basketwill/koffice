/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2.
 
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
 
   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __ROOTELEMENT_H
#define __ROOTELEMENT_H

#include <qpoint.h>

#include "complexelement.h"

class SequenceElement;


/**
 * A nice graphical root.
 */
class RootElement : public ComplexElement {
public:

    enum { contentPos, indexPos };

    RootElement(BasicElement* parent = 0);
    ~RootElement();

    /**
     * Calculates our width and height and
     * our children's parentPosition.
     */
    virtual void calcSizes(ContextStyle& style, int parentSize);

    /**
     * Draws the whole element including its children.
     * The `parentOrigin' is the point this element's parent starts.
     * We can use our parentPosition to get our own origin then.
     */
    virtual void draw(QPainter& painter, ContextStyle& style,
                      int parentSize, const QPoint& parentOrigin);

    /**
     * Enters this element while moving to the left starting inside
     * the element `from'. Searches for a cursor position inside
     * this element or to the left of it.
     */
    virtual void moveLeft(FormulaCursor* cursor, BasicElement* from);

    /**
     * Enters this element while moving to the right starting inside
     * the element `from'. Searches for a cursor position inside
     * this element or to the right of it.
     */
    virtual void moveRight(FormulaCursor* cursor, BasicElement* from);

    /**
     * Enters this element while moving up starting inside
     * the element `from'. Searches for a cursor position inside
     * this element or above it.
     */
    virtual void moveUp(FormulaCursor* cursor, BasicElement* from);

    /**
     * Enters this element while moving down starting inside
     * the element `from'. Searches for a cursor position inside
     * this element or below it.
     */
    virtual void moveDown(FormulaCursor* cursor, BasicElement* from);

    /**
     * Reinserts the index if it has been removed.
     */
    virtual void insert(FormulaCursor*, QList<BasicElement>&, Direction);
    
    /**
     * Removes all selected children and returns them. Places the
     * cursor to where the children have been.
     *
     * We remove ourselve if we are requested to remove our numerator.
     *
     * It is possible to remove the denominator. But after this we
     * are senseless and the caller is required to replace us.
     */
    virtual void remove(FormulaCursor*, QList<BasicElement>&, Direction);

    /**
     * Moves the cursor to a normal place where new elements
     * might be inserted.
     */
    virtual void normalize(FormulaCursor*, Direction);
    
    // main child
    //
    // If an element has children one has to become the main one.
    
    virtual SequenceElement* getMainChild();
    virtual void setMainChild(SequenceElement*);

    /**
     * Sets the cursor to select the child. The mark is placed before,
     * the position behind it.
     */
    virtual void selectChild(FormulaCursor* cursor, BasicElement* child);

    bool hasIndex() const { return index != 0; }
    
    // ComplexElement
    
    // Moves the cursor inside the index. The index has to exist.
    virtual void moveToUpperIndex(FormulaCursor*, Direction);
    virtual void moveToLowerIndex(FormulaCursor*, Direction) {}

    // Sets the cursor to point to the place where the index normaly
    // is. These functions are only used if there is no such index and
    // we want to insert them.
    virtual void setToUpperIndex(FormulaCursor*);
    virtual void setToLowerIndex(FormulaCursor*) {}

    // Tells whether we own those indexes
    virtual bool hasUpperIndex() const { return hasIndex(); }
    virtual bool hasLowerIndex() const { return false; }

    // Tells whether we are allowed to own them.
    // A root will never have a lower index!
    virtual bool mightHaveLowerIndex() const { return false; }
   
    virtual QDomElement getElementDom(QDomDocument *doc);
     
    
private:

    /**
     * The one below the graph.
     */
    SequenceElement* content;

    /**
     * An optional index.
     */
    SequenceElement* index;

    /**
     * The point the artwork relates to.
     */
    QPoint rootOffset;
};


#endif // __ROOTELEMENT_H
