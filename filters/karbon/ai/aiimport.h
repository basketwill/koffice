/* This file is part of the KDE project
   Copyright (C) 2002, Dirk Schönberger <dirk.schoenberger@sz-online.de>

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

#ifndef __AIIMPORT_H__
#define __AIIMPORT_H__

#include <koFilter.h>

#include "karbonaiparserbase.h"

class QDomElement;
class QTextStream;

class AiImport : public KoFilter, KarbonAIParserBase
{
	Q_OBJECT

public:
	AiImport( KoFilter* parent, const char* name, const QStringList& );
	virtual ~AiImport();

	virtual KoFilter::ConversionStatus convert( const QCString& from, const QCString& to );
private:
   QString m_result;

protected:
  void gotStartTag (const char *tagName, Parameters& params);
  void gotEndTag (const char *tagName);
  void gotSimpleTag (const char *tagName, Parameters& params);

  void parsingStarted();
  void parsingFinished();
  void parsingAborted();

};

#endif


