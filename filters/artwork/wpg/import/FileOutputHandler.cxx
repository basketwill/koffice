/* libwpg
 * Copyright (C) 2006 Fridrich Strba (fridrich.strba@bluewin.ch)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the 
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, 
 * Boston, MA  02111-1301 USA
 *
 * For further information visit http://libwpg.sourceforge.net
 */

/* "This product is not manufactured, approved, or supported by
 * Corel Corporation or Corel Corporation Limited."
 */

#include <string.h>
#include "FileOutputHandler.hxx"

FileOutputHandler::FileOutputHandler(std::ostringstream &contentStream) :
	mbIsTagOpened(false),
	mContentStream(contentStream)
{
}

void FileOutputHandler::startDocument()
{
}

void FileOutputHandler::startElement(const char *psName, const RVNGPropertyList &xPropList)
{
	if (mbIsTagOpened)
	{
		mContentStream << ">";
		mbIsTagOpened = false;
	}
	mContentStream << "<" << psName;

	RVNGPropertyList::Iter i(xPropList);
	for (i.rewind(); i.next(); )
	{
		// filter out libwpd elements
		if (strncmp(i.key(), "libwpd", 6) != 0)
			mContentStream << " " <<  i.key() << "=\"" << i()->getStr().cstr() << "\"";
	}
	mbIsTagOpened = true;
	msOpenedTagName = psName;
}

void FileOutputHandler::endElement(const char *psName)
{
	if (mbIsTagOpened)
	{
		if( msOpenedTagName == psName )
		{
			mContentStream << "/>";
			mbIsTagOpened = false;
		}
		else // should not happen, but handle it
		{
			mContentStream << ">";
			mContentStream << "</" << psName << ">";
			mbIsTagOpened = false;
		}
	}
	else
	{
		mContentStream << "</" << psName << ">";
		mbIsTagOpened = false;
	}
}

void FileOutputHandler::characters(const RVNGString &sCharacters)
{
	if (mbIsTagOpened)
	{
		mContentStream << ">";
		mbIsTagOpened = false;
	}
	mContentStream << sCharacters.cstr();
}

void FileOutputHandler::endDocument()
{
	if (mbIsTagOpened)
	{
		mContentStream << ">";
		mbIsTagOpened = false;
	}
}
