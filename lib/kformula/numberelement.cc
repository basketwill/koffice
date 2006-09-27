/* This file is part of the KDE project
   Copyright (C) 2006 Alfredo Beaumont Sainz <alfredo.beaumont@gmail.com>

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
 * Boston, MA 02110-1301, USA.
*/

#include <klocale.h>

#include "numberelement.h"
#include "kformuladefs.h"
#include "textelement.h"
#include "identifierelement.h"
#include "kformulacommand.h"
#include "kformulacontainer.h"
#include "formulaelement.h"
#include "creationstrategy.h"

KFORMULA_NAMESPACE_BEGIN

NumberElement::NumberElement( BasicElement* parent ) : TokenElement( parent ) {}

/*
 * Token elements' content has to be of homogeneous type. Every token element
 * must (TODO: check this) appear inside a non-token sequence, and thus, if
 * the command asks for a different content, a new element has to be created in
 * parent sequence.
 */
KCommand* NumberElement::buildCommand( Container* container, Request* request )
{
    FormulaCursor* cursor = container->activeCursor();
    if ( cursor->isReadOnly() ) {
        formula()->tell( i18n( "write protection" ) );
        return 0;
    }

    if ( *request == req_addNumber ) {
        KFCReplace* command = new KFCReplace( i18n("Add Number"), container );
        NumberRequest* nr = static_cast<NumberRequest*>( request );
        TextElement* element = creationStrategy->createTextElement( nr->ch(), false );
        command->addElement( element );
        return command;
    }

    if ( countChildren() == 0 || cursor->getPos() == countChildren() ) {
        // We are in the last position, so it's easy, call the parent to 
        // create a new child
        uint pos = static_cast<SequenceElement*>(getParent())->childPos( this );
        cursor->setTo( getParent(), pos + 1);
        return getParent()->buildCommand( container, request );
    }
    if ( cursor->getPos() == 0 ) {
        uint pos = static_cast<SequenceElement*>(getParent())->childPos( this );
        cursor->setTo( getParent(), pos );
        return getParent()->buildCommand( container, request );
    }

    // We are in the middle of a token, so:
    // a) Cut from mark to the end
    // b) Create a new token and add an element from key pressed
    // c) Create a new token and add elements cut previously
    // d) Move cursor to parent so that it command execution works fine

    switch( *request ) {
    case req_addTextChar: {
        KFCSplitToken* command = new KFCSplitToken( i18n("Add Text"), container );
        TextCharRequest* tr = static_cast<TextCharRequest*>( request );
        IdentifierElement* id = creationStrategy->createIdentifierElement();
        TextElement* text = creationStrategy->createTextElement( tr->ch() );
        command->addCursor( cursor );
        command->addToken( id );
        command->addContent( id, text );
        cursor->setTo( getParent(), static_cast<SequenceElement*>(getParent())->childPos( this ) + 1 );
        return command;
    }

    case req_addText:
    case req_addOperator:
    case req_addEmptyBox:
    case req_addNameSequence:
    case req_addBracket:
    case req_addSpace:
    case req_addFraction:
    case req_addRoot:
    case req_addSymbol:
    case req_addOneByTwoMatrix:
    case req_addMatrix: {
        uint pos = static_cast<SequenceElement*>(getParent())->childPos( this );
        cursor->setTo( getParent(), pos + 1);
        return getParent()->buildCommand( container, request );
    }
    default:
        return SequenceElement::buildCommand( container, request );
    }
    return 0;
}


KFORMULA_NAMESPACE_END
