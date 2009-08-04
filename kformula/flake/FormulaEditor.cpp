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

#include "FormulaEditor.h"
#include "BasicElement.h"
#include "RowElement.h"
#include "FixedElement.h"
#include "EmptyElement.h"
#include "NumberElement.h"
#include "ElementFactory.h"
#include "OperatorElement.h"
#include "IdentifierElement.h"
#include "ElementFactory.h"
#include "FormulaCommand.h"
#include <QPainter>
#include <QPen>
#include <algorithm>

#include <kdebug.h>
#include <QUndoCommand>
#include <KoOdfLoadingContext.h>
#include <KoOdfStylesReader.h>

FormulaEditor::FormulaEditor( FormulaCursor cursor, FormulaData* data )
{
    m_cursor=cursor;
    m_data=data;
}

FormulaEditor::FormulaEditor ( FormulaData* data )
{
    m_cursor=FormulaCursor(data->formulaElement(),0);
    m_data=data;
}



void FormulaEditor::paint( QPainter& painter ) const
{
    m_cursor.paint(painter);
}

FormulaCommand* FormulaEditor::insertText( const QString& text )
{
    FormulaCommand *undo = 0;
    m_inputBuffer = text;
    if (m_cursor.insideToken()) {
        TokenElement* token=static_cast<TokenElement*>(m_cursor.currentElement());
        if (m_cursor.hasSelection()) {
            undo=new FormulaCommandReplaceText(token,m_cursor.selection().first,m_cursor.selection().second-m_cursor.selection().first,text);
        } else {
            undo=new FormulaCommandReplaceText(token,m_cursor.position(),0,text);
        }
    } else {
        TokenElement* token = static_cast<TokenElement*>
            (ElementFactory::createElement(tokenType(text[0]),0));
        token->insertText(0,text);
        undo=insertElement(token);
        if (undo) {
            undo->setRedoCursorPosition(FormulaCursor(token,text.length()));
        }
    }
    if (undo) {
        undo->setText("Add text");
    }
    return undo;
}

FormulaCommand* FormulaEditor::insertData( const QString& data )
{
    FormulaElement* formulaElement = new FormulaElement();     // create a new root element
    KoOdfStylesReader stylesReader;
    KoOdfLoadingContext odfContext( stylesReader, 0 );
    // setup a DOM structure and start the actual loading process
    KoXmlDocument tmpDocument;
    tmpDocument.setContent( QString(data), false, 0, 0, 0 );
    BasicElement* element=ElementFactory::createElement(tmpDocument.documentElement().tagName(),0);
    element->readMathML( tmpDocument.documentElement() );     // and load the new formula
    return insertElement( element );
    //FIXME: we leak memory of the insert fails
}

FormulaCommand* FormulaEditor::insertElement( BasicElement* element )
{
    FormulaCommand *undo = 0;
    if (m_cursor.insideInferredRow()) {
        RowElement* tmprow=static_cast<RowElement*>(m_cursor.currentElement());
        QList<BasicElement*> list;
        list<<element;
        if (m_cursor.hasSelection()) {
            undo=new FormulaCommandReplaceElements(tmprow,m_cursor.selection().first,m_cursor.selection().second-m_cursor.selection().first,list,true);
        } else {
            undo=new FormulaCommandReplaceElements(tmprow,m_cursor.position(),0,list,false);
        }
/*    } else if (m_cursor.insideFixedElement()) {
        if (m_cursor.hasSelection()) {
            //TODO
            return 0;
        } else {
            BasicElement* tmpRow = new RowElement(m_cursor.currentElement());
            BasicElement* oldchild;
            BasicElement* oldpar;
            if (m_cursor.currentElement()->elementAfter(m_cursor.position())!=0) {
                oldchild=m_cursor.currentElement()->elementAfter(m_cursor.position());
                tmpRow->insertChild(0,element);
                tmpRow->insertChild(1,oldchild);
            } else {
                oldchild=m_cursor.currentElement()->elementBefore(m_cursor.position());
                tmpRow->insertChild(0,oldchild);
                tmpRow->insertChild(1,element);
            }
            oldchild->setParentElement(tmpRow);
            element->setParentElement(tmpRow);
            undo=new FormulaCommandWrapSingleElement(m_cursor.currentElement(),oldchild,tmpRow, tmpRow);
        }*/
    } else if (m_cursor.insideEmptyElement()) {
        undo=new FormulaCommandReplaceSingleElement(m_cursor.currentElement()->parentElement(),m_cursor.currentElement(),element);
    }
    if (undo) {
        undo->setText("Insert Element");
    }
    return undo;
}

FormulaCommand* FormulaEditor::remove( bool elementBeforePosition )
{
    FormulaCommand *undo=0;
    if (m_cursor.insideInferredRow()) {
        RowElement* tmprow=static_cast<RowElement*>(m_cursor.currentElement());
        if (m_cursor.isSelecting()) {
            undo=new FormulaCommandReplaceElements(tmprow,m_cursor.selection().first,m_cursor.selection().second-m_cursor.selection().first,QList<BasicElement*>());
        } else {
            if (elementBeforePosition && !m_cursor.isHome()) {
                undo=new FormulaCommandReplaceElements(tmprow,m_cursor.position()-1,1,QList<BasicElement*>());
            } else if (!elementBeforePosition && !m_cursor.isEnd()) {
                undo=new FormulaCommandReplaceElements(tmprow,m_cursor.position(),1,QList<BasicElement*>());
            }
        }
        if (tmprow->length()==0) {
            //TODO: this is not right I think...
            BasicElement* empty=new EmptyElement(tmprow);
            tmprow->insertChild(0,empty);
        }
    } else if (m_cursor.insideToken()) {
        TokenElement* tmptoken=static_cast<TokenElement*>(m_cursor.currentElement());
        if (m_cursor.hasSelection()) {
            undo=new FormulaCommandReplaceText(tmptoken,m_cursor.selection().first,m_cursor.selection().second-m_cursor.selection().first,"");
        } else {
            if (elementBeforePosition && !m_cursor.isHome()) {
                undo=new FormulaCommandReplaceText(tmptoken,m_cursor.position()-1,1,"");
            } else if (!elementBeforePosition && !m_cursor.isEnd()) {
                undo=new FormulaCommandReplaceText(tmptoken,m_cursor.position(),1,"");
            }
        }
/*    } else if (m_cursor.insideFixedElement()) {
        FixedElement* tmpfixed=static_cast<FixedElement*>(m_cursor.currentElement());
        if (m_cursor.hasSelection()) {
            foreach (BasicElement* tmp, tmpfixed->elementsBetween(m_cursor.selection().first,m_cursor.selection().second)) {
                BasicElement* newelement=new EmptyElement(m_cursor.currentElement());
                undo=new FormulaCommandReplaceSingleElement(tmpfixed,tmp,newelement);
            }
        } else {
            BasicElement* newelement=new EmptyElement(m_cursor.currentElement());
            if (elementBeforePosition && (m_cursor.currentElement()->elementBefore(m_cursor.position())!=0)) {
                undo=new FormulaCommandReplaceSingleElement(tmpfixed,m_cursor.currentElement()->elementBefore(m_cursor.position()),newelement);
            } else if (!elementBeforePosition && (m_cursor.currentElement()->elementAfter(m_cursor.position())!=0)) {
                undo=new FormulaCommandReplaceSingleElement(tmpfixed,m_cursor.currentElement()->elementAfter(m_cursor.position()),newelement);
            }
        }*/
    }
    if (undo) {
        undo->setText("Remove stuff");
    }
    return undo;
}

void FormulaEditor::setData ( FormulaData* data )
{
    m_data=data;
}


FormulaData* FormulaEditor::formulaData() const
{
    return m_data;
}

QString FormulaEditor::inputBuffer() const
{
    return m_inputBuffer;
}

QString FormulaEditor::tokenType ( const QChar& character ) const
{
    QChar::Category chat=character.category();
    if (character.isNumber()) {
        return "mn";
    }
    else if (chat==QChar::Punctuation_Connector ||
             chat==QChar::Punctuation_Dash ||
             chat==QChar::Punctuation_Open ||
             chat==QChar::Punctuation_Close ||
             chat==QChar::Punctuation_InitialQuote ||
             chat==QChar::Punctuation_FinalQuote ||
             chat==QChar::Symbol_Math) {
        return "mo";
    }
    else if (character.isLetter()) {
        return "mi";
    }
    return "mi";
}


FormulaCursor& FormulaEditor::cursor() 
{
    return m_cursor;
}

void FormulaEditor::setCursor ( FormulaCursor& cursor )
{
    m_cursor=cursor;
}

