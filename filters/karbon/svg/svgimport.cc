/* This file is part of the KDE project
   Copyright (C) 2002, Sven L�ppken <sven@kde.org>

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

#include <svgimport.h>
#include <koFilterChain.h>
#include <kgenericfactory.h>
#include <kdebug.h>
#include <koUnit.h>
#include <koGlobal.h>
#include <shapes/vellipse.h>
#include <shapes/vrectangle.h>
#include <shapes/vpolygon.h>
#include <qcolor.h>
#include <qfile.h>
#include <qregexp.h>
#include <iostream>

typedef KGenericFactory<SvgImport, KoFilter> SvgImportFactory;
K_EXPORT_COMPONENT_FACTORY( libkarbonsvgimport, SvgImportFactory( "karbonsvgimport" ) );

SvgImport::SvgImport(KoFilter *, const char *, const QStringList&) :
    KoFilter(),
    outdoc( "DOC" )
{
}

SvgImport::~SvgImport()
{

}

KoFilter::ConversionStatus SvgImport::convert(const QCString& from, const QCString& to)
{
	// check for proper conversion
	if( to != "application/x-karbon" || from != "image/svg+xml" )
		return KoFilter::NotImplemented;

	QFile in( m_chain->inputFile() );
    if( !in.open( IO_ReadOnly ) )
	{
		kdError(30502) << "Unable to open input file" << endl;
		in.close();
		return KoFilter::FileNotFound;
	}

	QTextStream stream( &in );
	inpdoc.setContent( stream.device() );

	// Do the conversion!

	convert();

	KoStoreDevice* out = m_chain->storageFile( "root", KoStore::Write );
	if( !out ) 
	{
		kdError(30502) << "Unable to open output file!" << endl;
		return KoFilter::StorageCreationError;
	}
	QCString cstring = outdoc.toCString(); // utf-8 already
	out->writeBlock( cstring.data(), cstring.length() );

	in.close();
	return KoFilter::OK; // was successfull
}

void
SvgImport::convert()
{
	QDomElement docElem = inpdoc.documentElement();
	// undo y-mirroring
	GraphicsContext *gc = new GraphicsContext;
	gc->matrix.scale( 1, -1 );
	gc->matrix.translate( 0, -841 );
	m_gc.push( gc );
	parseGroup( 0L, docElem );

	outdoc = m_document.saveXML();
}

void
SvgImport::parseStyle( VObject *obj, const QDomElement &e )
{
	QColor c;
	GraphicsContext *gc = new GraphicsContext;
	// set as default
	if( m_gc.current() )
		*gc = *( m_gc.current() );

	VColor strokecolor	= gc->stroke.color();
	VColor fillcolor	= gc->fill.color();

	// try normal PA
	if( !e.attribute( "fill" ).isEmpty() )
	{
		if( e.attribute( "fill" ) == "none" )
			gc->fill.setType( VFill::none );
		else
		{
			c.setNamedColor( e.attribute( "fill" ) );
			fillcolor.set( c.red() / 255.0, c.green() / 255.0, c.blue() / 255.0 );
			gc->fill.setType( VFill::solid );
		}
	}
	if( !e.attribute( "fill-rule" ).isEmpty() )
		if( e.attribute( "fill-rule" ) == "nonzero" )
			gc->fill.setFillRule( VFill::winding );
		else if( e.attribute( "fill-rule" ) == "evenodd" )
			gc->fill.setFillRule( VFill::evenOdd );

	if( !e.attribute( "stroke" ).isEmpty() )
	{
		if( e.attribute( "stroke" ) == "none" )
			gc->stroke.setType( VStroke::none );
		else
		{
			c.setNamedColor( e.attribute( "stroke" ) );
			strokecolor.set( c.red() / 255.0, c.green() / 255.0, c.blue() / 255.0 );
			gc->stroke.setType( VStroke::solid );
		}
	}
	if( !e.attribute( "stroke-width" ).isEmpty() )
		gc->stroke.setLineWidth( e.attribute( "stroke-width" ).toDouble() );
	// handle opacity
	if( !e.attribute( "stroke-opacity" ).isEmpty() )
		strokecolor.setOpacity( e.attribute( "stroke-opacity" ).toFloat() );
	else if( !e.attribute( "fill-opacity" ).isEmpty() )
		fillcolor.setOpacity( e.attribute( "fill-opacity" ).toFloat() );
	if( !e.attribute( "opacity" ).isEmpty() )
	{
		fillcolor.setOpacity( e.attribute( "opacity" ).toFloat() );
		strokecolor.setOpacity( e.attribute( "opacity" ).toFloat() );
	}

	// try style attr
	QString style = e.attribute( "style" ).simplifyWhiteSpace();
	QStringList substyles = QStringList::split( ';', style );
    for( QStringList::Iterator it = substyles.begin(); it != substyles.end(); ++it )
	{
		QStringList substyle = QStringList::split( ':', (*it) );
		QString command	= substyle[0].stripWhiteSpace();
		QString params	= substyle[1].stripWhiteSpace();
		if( command == "fill" )
		{
			if( params == "none" )
				gc->fill.setType( VFill::none );
			else
			{
				c.setNamedColor( params );
				fillcolor.set( c.red() / 255.0, c.green() / 255.0, c.blue() / 255.0 );
				gc->fill.setType( VFill::solid );
			}
		}
		else if( command == "fill-rule" )
		{
			if( params == "fill-rule" )
				gc->fill.setFillRule( VFill::winding );
			else if( params == "fill-rule" )
				gc->fill.setFillRule( VFill::evenOdd );
		}
		else if( command == "stroke" )
		{
			if( params == "none" )
				gc->stroke.setType( VStroke::none );
			else
			{
				c.setNamedColor( params );
				strokecolor.set( c.red() / 255.0, c.green() / 255.0, c.blue() / 255.0 );
				gc->stroke.setType( VStroke::solid );
			}
		}
		else if( command == "stroke-width" )
			gc->stroke.setLineWidth( params.toDouble() );
		// handle opacity
		else if( command == "stroke-opacity" )
			strokecolor.setOpacity( params.toFloat() );
		else if( command == "fill-opacity" )
			fillcolor.setOpacity( params.toFloat() );
		else if( command == "opacity" )
		{
			fillcolor.setOpacity( params.toFloat() );
			strokecolor.setOpacity( params.toFloat() );
		}
	}

	gc->fill.setColor( fillcolor );
	gc->stroke.setColor( strokecolor );
	obj->setFill( gc->fill );
	obj->setStroke( gc->stroke );
	obj->transform( gc->matrix );
	m_gc.push( gc );
}

void
SvgImport::parseGroup( VGroup *grp, const QDomElement &e )
{
	QDomElement b = e.firstChild().toElement();
	for( ; !b.isNull(); b = b.nextSibling().toElement() )
	{
		VObject *obj = 0L;
		if( b.tagName() == "g" )
		{
			VGroup *group = new VGroup( grp ? grp : &m_document );
			parseStyle( group, b );
			parseGroup( group, b );
			if( grp )
				grp->append( group );
			else
				m_document.append( group );
			m_gc.pop();
			continue;
		}
		else if( b.tagName() == "rect" )
		{
			int x = b.attribute( "x" ).toInt();
			int y = b.attribute( "y" ).toInt();
			int width = b.attribute( "width" ).toInt();
			int height = b.attribute( "height" ).toInt();
			obj = new VRectangle( 0L, KoPoint( x, height + y ) , width, height );
		}
		else if( b.tagName() == "ellipse" )
		{
			double rx = b.attribute( "rx" ).toDouble();
			double ry = b.attribute( "ry" ).toDouble();
			double left	= b.attribute( "cx" ).toDouble() - ( rx / 2.0 );
			double top	= b.attribute( "cy" ).toDouble() + ( ry / 2.0 );
			// Append the ellipse to the document
			obj = new VEllipse( 0L, KoPoint( left, top ), rx * 2.0, ry * 2.0 );
		}
		else if( b.tagName() == "circle" )
		{
			double r = b.attribute( "r" ).toDouble();
			double left	= b.attribute( "cx" ).toDouble() - ( r / 2.0 );
			double top	= b.attribute( "cy" ).toDouble() + ( r / 2.0 );
			// Append the ellipse to the document
			obj = new VEllipse( 0L, KoPoint( left, top ), r * 2.0, r * 2.0 );
		}
		else if( b.tagName() == "line" )
		{
			VComposite *path = new VComposite( &m_document );
			double x1 = b.attribute( "x1" ).isEmpty() ? 0.0 : b.attribute( "x1" ).toDouble();
			double y1 = b.attribute( "y1" ).isEmpty() ? 0.0 : b.attribute( "y1" ).toDouble();
			double x2 = b.attribute( "x2" ).isEmpty() ? 0.0 : b.attribute( "x2" ).toDouble();
			double y2 = b.attribute( "y2" ).isEmpty() ? 0.0 : b.attribute( "y2" ).toDouble();
			path->moveTo( KoPoint( x1, y1 ) );
			path->lineTo( KoPoint( x2, y2 ) );
			obj = path;
		}
		else if( b.tagName() == "polyline" || b.tagName() == "polygon" )
		{
			VComposite *path = new VComposite( &m_document );
			bool bFirst = true;

			QString points = b.attribute( "points" ).simplifyWhiteSpace();
			points.replace( QRegExp( "," ), " " );
			points.replace( QRegExp( "\r" ), "" );
		    points.replace( QRegExp( "\n" ), "" );
			QStringList pointList = QStringList::split( ' ', points );
			for( QStringList::Iterator it = pointList.begin(); it != pointList.end(); it++ )
			{
				if( bFirst )
				{
					path->moveTo( KoPoint( (*(it++)).toDouble(), (*it).toDouble() ) );
					bFirst = false;
				}
				else
					path->lineTo( KoPoint( (*(it++)).toDouble(), (*it).toDouble() ) );
			}
			if( b.tagName() == "polygon" ) path->close();
			obj = path;
		}
		parseStyle( obj, b );
		if( grp )
			grp->append( obj );
		else
			m_document.append( obj );
		m_gc.pop();
	}
}

#include <svgimport.moc>
