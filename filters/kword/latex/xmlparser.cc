/*
** A program to convert the XML rendered by KWord into LATEX.
**
** Copyright (C) 2000 Robert JACOLIN
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

#include <kdebug.h>

#include "xmlparser.h"

/* Init static data */
FileHeader* XmlParser::_fileHeader = 0;

XmlParser::XmlParser(const char *data)
{
	_document     = data;
	_index        = 0;
	_childCurrent = 0;

	// Get the tree
	_arbreXml     = ParseXml(_document, &_index);
	_tokenCurrent = _arbreXml;
	//PrintXml(_arbreXml, 3);
}

XmlParser::XmlParser()
{
	_document = 0;
	_index    = 0;
	_arbreXml = 0;
	_tokenCurrent = 0;
	_childCurrent = 0;
}

XmlParser::~XmlParser()
{
	kdDebug() << "destruction of XmlParser (tree)" << endl;
	delete _arbreXml;
}

const char *XmlParser::getDocument() const
{
	return _document;
}

Markup* XmlParser::getNextMarkup()
{
	Markup *m= 0;
	// Lit la zone de donnee _document et retourne un bloc.
	kdDebug() << "markup suivant" << endl;
	if(_tokenCurrent == 0)
		return 0;

	while((_tokenCurrent->eType == TT_Space || _tokenCurrent->eType == TT_EOL) 
		&& _tokenCurrent->pNext != 0)
	{
		_tokenCurrent= _tokenCurrent->pNext;
	}
		
	if(_tokenCurrent->eType == TT_Markup)
	{
		m = (Markup*) _tokenCurrent;
		_tokenCurrent = _tokenCurrent->pNext;
		kdDebug() << "getMarkup : " << m->token.zText << endl;

	}
	kdDebug() << "ok" << endl;
	return m;
}

Token * XmlParser::getNextChild()
{
	if(_childCurrent == NULL)
		_childCurrent = ((Markup*) _tokenCurrent)->pContent;
	else
		_childCurrent = _childCurrent->pNext;
	return _childCurrent;
}

Token* XmlParser::enterTokenChild(const Markup *m)
{	
	Token *s;
	
	s = _tokenCurrent;
	_tokenCurrent= m->pContent;
	return s;
}

