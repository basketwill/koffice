/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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

#include <qprinter.h> // FIXME: is this needed ? 
#include <qdatetm.h> // FIXME: what's this ?
#include <qpainter.h>
#include <qrect.h>

#include <kapp.h> // FIXME: is this needed ?
#include <kimgio.h> // FIXME: remove this
#include <kstddirs.h>

/*
#include <komlParser.h>
#include <komlStreamFeed.h>
#include <komlWriter.h>
#include <komlMime.h>
*/

#include <koDocument.h>
#include <koPageLayoutDia.h>
#include <koQueryTypes.h>

#include "kimage_doc.h"
#include "kimage_shell.h"
#include "kimage_factory.h"

#define MM_TO_POINT 2.83465
#define POINT_TO_MM 0.3527772388

KImageDocument::KImageDocument( KoDocument* parent, const char* name )
  : KoDocument( parent, name )
{
  // FIXME: move this to main() or init_libkimage()
  kimgioRegister();
}

KImageDocument::~KImageDocument()
{
  m_bEmpty = true;
  //m_bModified = false;

  m_leftBorder = 20.0;
  m_rightBorder = 20.0;
  m_topBorder = 20.0;
  m_bottomBorder = 20.0;
  //m_paperFormat = PG_DIN_A4;
  m_paperWidth = PG_A4_WIDTH;
  m_paperHeight = PG_A4_HEIGHT;
  calcPaperSize();
  //m_orientation = PG_PORTRAIT;
}

bool KImageDocument::initDoc()
{

  return true;
}

View* KImageDocument::createView( QWidget* parent, const char* name )
{
  KImageView* view = new KImageView( this, parent, name );
  addView( view );

  return view;
}

Shell* KImageDocument::createShell()
{
  Shell* shell = new KImageShell;
  shell->setRootPart( this );
  shell->show();

  return shell;
}

void KImageDocument::paintContent( QPainter& /* _painter */, const QRect& /* _rect */, bool /* _transparent */ )
{
  // TODO : paint image here
  // draw only the rect given in _rect
  
  //paintPixmap( &_painter, _rect);
}

QCString KImageDocument::mimeType() const
{
  return "application/x-kimage";
}

QString KImageDocument::configFile() const
{
    return readConfigFile( locate("kim", "kimage.rc", KImageFactory::global()) );
}

/*
bool KImageDocument::save( ostream& out, const char* / * format * / )
{
  out << "<?xml version=\"1.0\"?>" << endl;
  out << otag << "<DOC author=\"" << "Torben Weis" << "\" email=\"" << "weis@kde.org" << "\" editor=\"" << "kimage"
      << "\" mime=\"" << "application/x-kimage" << "\" >" << endl;

  out << otag << "<PAPER format=\"" << paperFormatString().ascii() << "\" orientation=\"" << orientationString().ascii() << "\">" << endl;
  out << indent << "<PAPERBORDERS left=\"" << leftBorder() << "\" top=\"" << topBorder() << "\" right=\"" << rightBorder()
      << " bottom=\"" << bottomBorder() << "\"/>" << endl;
  out << indent << "<HEAD left=\"" << headLeft().ascii() << "\" center=\"" << headMid().ascii() << "\" right=\"" << headRight().ascii() << "\"/>" << endl;
  out << indent << "<FOOT left=\"" << footLeft().ascii() << "\" center=\"" << footMid().ascii() << "\" right=\"" << footRight().ascii() << "\"/>" << endl;
  out << etag << "</PAPER>" << endl;

  out << etag << "</DOC>" << endl;

  setModified( FALSE );

  return true;
}
*/

/*
bool KImageDocument::completeSaving( KOStore::Store_ptr _store )
{
  QString u = url();
  u += "/image";

  if ( _store->open( u, "image/bmp" ) )
  {
    ostorestream out( _store );
    out << m_image;
    out.flush();
    _store->close();
  }

  return true;
}
*/

/*
bool KImageDocument::loadBinary( istream& _stream, bool _randomaccess, KOStore::Store_ptr _store )
{
  kdebug( KDEBUG_INFO, 0, "------------------------ LOADING --------------------" );

  // implement binary loading here.  

  kdebug( KDEBUG_INFO, 0, "--------------------- LOADING DONE ------------------" );
}
*/

/*
bool KImageDocument::loadXML( KOMLParser& parser, KOStore::Store_ptr )
{
  kdebug( KDEBUG_INFO, 0, "------------------------ LOADING --------------------" );

  string tag;
  vector<KOMLAttrib> lst;
  string name;

  // DOC
  if ( !parser.open( "DOC", tag ) )
  {
    kdebug( KDEBUG_INFO, 0, "Missing DOC" );
    return false;
  }

  KOMLParser::parseTag( tag.c_str(), name, lst );
  vector<KOMLAttrib>::const_iterator it = lst.begin();
  for( ; it != lst.end(); it++ )
  {
    if ( it->m_strName == "mime" )
    {
      if ( it->m_strValue != "application/x-kimage" )
      {
		kdebug( KDEBUG_INFO, 0, "Unknown mime type %s", it->m_strValue.c_str() );
		return false;
      }
    }
  }

  QString format = "A4", orientation = "Portrait";
  float left = 20.0, right = 20.0, bottom = 20.0, top = 20.0;
  QString hl="", hm="", hr="";
  QString fl="", fm="", fr="";

  // PAPER
  while( parser.open( 0L, tag ) )
  {
    KOMLParser::parseTag( tag.c_str(), name, lst );

    if ( name == "PAPER" )
    {
      KOMLParser::parseTag( tag.c_str(), name, lst );
      vector<KOMLAttrib>::const_iterator it = lst.begin();
      for( ; it != lst.end(); it++ )
      {
		if ( it->m_strName == "format" )
		{
		  format = it->m_strValue.c_str();
		}
		else if ( it->m_strName == "orientation" )
		{
		  orientation = it->m_strValue.c_str();
		}
		else
		  kdebug( KDEBUG_INFO, 0, "Unknown attrib PAPER:'%s'",it->m_strName.c_str() );
      }

      // PAPERBORDERS, HEAD, FOOT
      while( parser.open( 0L, tag ) )
      {
		KOMLParser::parseTag( tag.c_str(), name, lst );

		if ( name == "PAPERBORDERS" )
		{
		  KOMLParser::parseTag( tag.c_str(), name, lst );
		  vector<KOMLAttrib>::const_iterator it = lst.begin();
		  for( ; it != lst.end(); it++ )
		  {
		    if ( it->m_strName == "left" )
		    {
		      left = atof( it->m_strValue.c_str() );
		    }
		    else if ( it->m_strName == "top" )
		    {
		      top = atof( it->m_strValue.c_str() );
		    }
		    else if ( it->m_strName == "right" )
		    {
		      right = atof( it->m_strValue.c_str() );
		    }
		    else if ( it->m_strName == "bottom" )
		    {
		      bottom = atof( it->m_strValue.c_str() );
		    }
		    else
		      kdebug( KDEBUG_INFO, 0, "Unknown attrib 'PAPERBORDERS:%s'", it->m_strName.c_str() );
		  }
		}
      	else if ( name == "HEAD" )
		{
		  KOMLParser::parseTag( tag.c_str(), name, lst );
		  vector<KOMLAttrib>::const_iterator it = lst.begin();
		  for( ; it != lst.end(); it++ )
		  {
		    if ( it->m_strName == "left" )
		    {
		      hl = it->m_strValue.c_str();
		    }
		    else if ( it->m_strName == "center" )
		    {
		      hm = it->m_strValue.c_str();
		    }
		    else if ( it->m_strName == "right" )
		    {
		      hr = it->m_strValue.c_str();
		    }
	        else
	          kdebug( KDEBUG_INFO, 0, "Unknown attrib 'HEAD:%s'", it->m_strName.c_str() );
	      }
	    }
        else if ( name == "FOOT" )
	    {
		  KOMLParser::parseTag( tag.c_str(), name, lst );
		  vector<KOMLAttrib>::const_iterator it = lst.begin();
		  for( ; it != lst.end(); it++ )
		  {
	 	    if ( it->m_strName == "left" )
	        {
	          fl = it->m_strValue.c_str();
	        }
	        else if ( it->m_strName == "center" )
	        {
	        	fm = it->m_strValue.c_str();
	        }
	        else if ( it->m_strName == "right" )
	        {
	          fr = it->m_strValue.c_str();
	        }
	        else
	          kdebug( KDEBUG_INFO, 0, "Unknown attrib 'FOOT:%s'", it->m_strName.c_str() );
	      }
	    }
	    else
		  kdebug( KDEBUG_INFO, 0, "Unknown tag '%s' in PAPER", tag.c_str() );
	
	    if ( !parser.close( tag ) )
        {
	      kdebug( KDEBUG_INFO, 0, "ERROR: Closing PAPER" );
	      return false;
	    }
      }
    }
    else
      kdebug( KDEBUG_INFO, 0, "Unknown tag '%s' in DOC", tag.c_str() );

    if ( !parser.close( tag ) )
    {
      kdebug( KDEBUG_INFO, 0, "ERROR: Closing DOC" );
      return false;
    }
  }

  parser.close( tag );

  setPaperLayout( left, top, right, bottom, format, orientation );
  setHeadFootLine( hl, hm, hr, fl, fm, fr );

  kdebug( KDEBUG_INFO, 0, "------------------------ LOADING DONE --------------------" );

  return true;
}
*/

/*
bool KImageDocument::completeLoading( KOStore::Store_ptr _store )
{
  kdebug( KDEBUG_INFO, 0, "------------------------ COMPLETION DONE --------------------" );

  QString u = url();
  u += "/image";
  if ( _store->open( u, "" ) )
  {
    istorestream in( _store );
    in >> m_image;
    _store->close();
  }

  m_bModified = false;
  m_bEmpty = false;

  emit sigUpdateView();

  return true;
}
*/

/*
void KImageDocument::print( QPaintDevice* _dev )
{
  QPainter painter;
  painter.begin( _dev );

  // HACK
  int pagenr = 1;
  QString m_strName = "";

  // print head line
  QFont font( "Times", 10 );
  painter.setFont( font );
  QFontMetrics fm = painter.fontMetrics();
  int w = fm.width( headLeft( pagenr, m_strName ) );
  if ( w > 0 )
    painter.drawText( (int)( MM_TO_POINT * leftBorder() ),
		      (int)( MM_TO_POINT * 10.0 ), headLeft( pagenr, m_strName ) );
  w = fm.width( headMid( pagenr, m_strName ) );
  if ( w > 0 )
    painter.drawText( (int)( MM_TO_POINT * leftBorder() +
			     ( MM_TO_POINT * printableWidth() - (float)w ) / 2.0 ),
		      (int)( MM_TO_POINT * 10.0 ), headMid( pagenr, m_strName ) );
  w = fm.width( headRight( pagenr, m_strName ) );
  if ( w > 0 )
    painter.drawText( (int)( MM_TO_POINT * leftBorder() +
			     MM_TO_POINT * printableWidth() - (float)w ),
		      (int)( MM_TO_POINT * 10.0 ), headRight( pagenr, m_strName ) );

  // print foot line
  w = fm.width( footLeft( pagenr, m_strName ) );
  if ( w > 0 )
    painter.drawText( (int)( MM_TO_POINT * leftBorder() ),
		      (int)( MM_TO_POINT * ( paperHeight() - 10.0 ) ),
		      footLeft( pagenr, m_strName ) );
  w = fm.width( footMid( pagenr, m_strName ) );
  if ( w > 0 )
    painter.drawText( (int)( MM_TO_POINT * leftBorder() +
			     ( MM_TO_POINT * printableWidth() - (float)w ) / 2.0 ),
		      (int)( MM_TO_POINT * ( paperHeight() - 10.0 ) ),
		      footMid( pagenr, m_strName ) );
  w = fm.width( footRight( pagenr, m_strName ) );
  if ( w > 0 )
    painter.drawText( (int)( MM_TO_POINT * leftBorder() +
			     MM_TO_POINT * printableWidth() - (float)w ),
		      (int)( MM_TO_POINT * ( paperHeight() - 10.0 ) ),
		      footRight( pagenr, m_strName ) );

  painter.translate( MM_TO_POINT * m_leftBorder, MM_TO_POINT * m_topBorder );

  // Print centered
  painter.drawImage( ( MM_TO_POINT * printableWidth() - m_image.width() ) / 2,
		     ( MM_TO_POINT * printableHeight() - m_image.height() ) / 2,
		     m_image );

  painter.end();
}
*/

/*
void KImageDocument::draw( QPaintDevice* _dev, long int _width, long int _height,
		      float _scale )
{
  kdebug( KDEBUG_INFO, 0, "DRAWING w=%li h=%li", _width, _height );

  QPainter painter;
  painter.begin( _dev );

  if ( _scale != 1.0 )
    painter.scale( _scale, _scale );

  // Print centered
  int x = ( _width - m_image.width() ) / 2;
  int y = ( _height - m_image.height() ) / 2;
  QPoint p( x, y );

  QRect rect;
  if ( x >= 0 )
  {
    rect.setLeft( 0 );
    rect.setWidth( m_image.width() );
  }
  else
  {
    rect.setLeft( -x );
    rect.setWidth( m_image.width() + 2*x );
  }
  if ( y >= 0 )
  {
    rect.setTop( 0 );
    rect.setHeight( m_image.height() );
  }
  else
  {
    rect.setTop( -x );
    rect.setHeight( m_image.height() + 2*x );
  }

  painter.drawImage( p, m_image, rect );

  painter.end();
}
*/

/*
void KImageDocument::paperLayoutDlg()
{
  KoPageLayout pl;
  pl.format = paperFormat();
  pl.orientation = orientation();
  pl.unit = PG_MM;
  pl.width = m_paperWidth;
  pl.height = m_paperHeight;
  pl.left = leftBorder();
  pl.right = rightBorder();
  pl.top = topBorder();
  pl.bottom = bottomBorder();

  KoHeadFoot hf;
  hf.headLeft = headLeft();
  hf.headRight = headRight();
  hf.headMid = headMid();
  hf.footLeft = footLeft();
  hf.footRight = footRight();
  hf.footMid = footMid();

  if ( !KoPageLayoutDia::pageLayout( pl, hf, FORMAT_AND_BORDERS | HEADER_AND_FOOTER ) )
    return;

  if ( pl.format == PG_CUSTOM )
  {
    m_paperWidth = pl.width;
    m_paperHeight = pl.height;
  }

  setPaperLayout( pl.left, pl.top, pl.right, pl.bottom, pl.format, pl.orientation );

  setHeadFootLine( hf.headLeft, hf.headMid, hf.headRight, hf.footLeft, hf.footMid, hf.footRight );

  emit sigUpdateView();
}

void KImageDocument::setHeadFootLine( const char *_headl, const char *_headm, const char *_headr,
				    const char *_footl, const char *_footm, const char *_footr )
{
  m_headLeft = _headl;
  m_headRight = _headr;
  m_headMid = _headm;
  m_footLeft = _footl;
  m_footRight = _footr;
  m_footMid = _footm;

  m_bModified = TRUE;
}

void KImageDocument::setPaperLayout( float _leftBorder, float _topBorder, float _rightBorder, float _bottomBorder,
				const char * _paper, const char* _orientation )
{
    KoFormat f = paperFormat();
    KoOrientation o = orientation();

    if ( strcmp( "A3", _paper ) == 0L )
	f = PG_DIN_A3;
    else if ( strcmp( "A4", _paper ) == 0L )
	f = PG_DIN_A4;
    else if ( strcmp( "A5", _paper ) == 0L )
	f = PG_DIN_A5;
    else if ( strcmp( "B5", _paper ) == 0L )
	f = PG_DIN_B5;
    else if ( strcmp( "Executive", _paper ) == 0L )
	f = PG_US_EXECUTIVE;
    else if ( strcmp( "Letter", _paper ) == 0L )
	f = PG_US_LETTER;
    else if ( strcmp( "Legal", _paper ) == 0L )
	f = PG_US_LEGAL;
    else if ( strcmp( "Screen", _paper ) == 0L )
	f = PG_SCREEN;
    else if ( strcmp( "Custom", _paper ) == 0L )
    {
      m_paperWidth = 0.0;
      m_paperHeight = 0.0;
      f = PG_CUSTOM;
      QString tmp( _paper );
      m_paperWidth = atof( _paper );
      int i = tmp.find( 'x' );
      if ( i != -1 )
	m_paperHeight = atof( tmp.data() + i + 1 );
      if ( m_paperWidth < 10.0 )
	m_paperWidth = PG_A4_WIDTH;
      if ( m_paperHeight < 10.0 )
	m_paperWidth = PG_A4_HEIGHT;
    }

    if ( strcmp( "Portrait", _orientation ) == 0L )
	o = PG_PORTRAIT;
    else if ( strcmp( "Landscape", _orientation ) == 0L )
	o = PG_LANDSCAPE;

    setPaperLayout( _leftBorder, _topBorder, _rightBorder, _bottomBorder, f, o );
}

void KImageDocument::setPaperLayout( float _leftBorder, float _topBorder, float _rightBorder, float _bottomBorder,
				   KoFormat _paper, KoOrientation _orientation )
{
  m_leftBorder = _leftBorder;
  m_rightBorder = _rightBorder;
  m_topBorder = _topBorder;
  m_bottomBorder = _bottomBorder;
  m_orientation = _orientation;
  m_paperFormat = _paper;

  calcPaperSize();

  m_bModified = TRUE;
}

QString KImageDocument::completeHeading( const char *_data, 
				    int / * _page * /, const char* / * _table * / )
{
  / * QString page;
    page.sprintf( "%i", _page );
    QString f = m_strFileURL.data();
    if ( f.isNull() )
	f = "";
    QString n = "";
    if ( f != "" )
    {
	KURL u( f.data() );
	n = u.filename();
	} * /
    QString t = QTime::currentTime().toString().copy();
    QString d = QDate::currentDate().toString().copy();

    QString tmp = _data;
    int pos = 0;
    //  while ( ( pos = tmp.find( "<file>", pos ) ) != -1 )
    //  tmp.replace( pos, 6, f.data() );/
    pos = 0;
    //  while ( ( pos = tmp.find( "<name>", pos ) ) != -1 )
    /7  tmp.replace( pos, 6, n.data() );
    pos = 0;
    while ( ( pos = tmp.find( "<time>", pos ) ) != -1 )
	tmp.replace( pos, 6, t.data() );
    pos = 0;
    while ( ( pos = tmp.find( "<date>", pos ) ) != -1 )
	tmp.replace( pos, 6, d.data() );
    pos = 0;
    while ( ( pos = tmp.find( "<author>", pos ) ) != -1 )
	tmp.replace( pos, 8, "??" );
    pos = 0;
    while ( ( pos = tmp.find( "<email>", pos ) ) != -1 )
	tmp.replace( pos, 7, "??" );

    return QString( tmp.data() );
}
*/

void KImageDocument::calcPaperSize()
{
/*
    switch( m_paperFormat )
    {
    case PG_DIN_A5:
        m_paperWidth = PG_A5_WIDTH;
	m_paperHeight = PG_A5_HEIGHT;
	break;
    case PG_DIN_A4:
	m_paperWidth = PG_A4_WIDTH;
	m_paperHeight = PG_A4_HEIGHT;
	break;
    case PG_DIN_A3:
	m_paperWidth = PG_A3_WIDTH;
	m_paperHeight = PG_A3_HEIGHT;
	break;
    case PG_DIN_B5:
	m_paperWidth = PG_B5_WIDTH;
	m_paperHeight = PG_B5_HEIGHT;
	break;
    case PG_US_EXECUTIVE:
	m_paperWidth = PG_US_EXECUTIVE_WIDTH;
	m_paperHeight = PG_US_EXECUTIVE_HEIGHT;
	break;
    case PG_US_LETTER:
	m_paperWidth = PG_US_LETTER_WIDTH;
	m_paperHeight = PG_US_LETTER_HEIGHT;
	break;
    case PG_US_LEGAL:
	m_paperWidth = PG_US_LEGAL_WIDTH;
	m_paperHeight = PG_US_LEGAL_HEIGHT;
	break;
    case PG_SCREEN:
        m_paperWidth = PG_SCREEN_WIDTH;
        m_paperHeight = PG_SCREEN_HEIGHT;
    case PG_CUSTOM:
        return;
    }
*/
}

/*
QString KImageDocument::paperFormatString()
{
  QString paperFormatStr;
  
  switch( m_paperFormat )
  {
    case PG_DIN_A5:
	  paperFormatStr = "A5";
	  break;
    case PG_DIN_A4:
	  paperFormatStr = "A4";
	  break;
    case PG_DIN_A3:
	  paperFormatStr = "A3";
	  break;
    case PG_DIN_B5:
	  paperFormatStr = "B5";
	  break;
    case PG_US_EXECUTIVE:
	  paperFormatStr = "Executive";
	  break;
    case PG_US_LETTER:
	  paperFormatStr = "Letter";
	  break;
    case PG_US_LEGAL:
	  paperFormatStr = "Legal";
	  break;
    case PG_SCREEN:
	  paperFormatStr = "Screen";
	  break;
    case PG_CUSTOM:
      QString tmp;
      tmp.sprintf( "%fx%f", m_paperWidth, m_paperHeight );
      paperFormatStr = tmp;
  }
  return paperFormatStr;
}

QString KImageDocument::orientationString()
{
  QString orientationStr;
  
  switch( m_orientation )
  {
    case QPrinter::Portrait :
      orientationStr = "Portrait";
      break;
    case QPrinter::Landscape :
      orientationStr = "Landscape";
      break;
  }
  return orientationStr;
}

bool KImageDocument::openDocument( const QString & _filename, const char *_format )
{
  if ( !m_image.load( _filename, _format ) )
    return false;

  if ( _format )
    m_strImageFormat = _format;
  else
    m_strImageFormat = QImage::imageFormat( _filename );

  emit sigUpdateView();

  m_bModified = true;
  m_bEmpty = false;

  return true;
}

bool KImageDocument::saveDocument( const QString & _filename, const char* / * _format * / )
{
  assert( !isEmpty() );

  return m_image.save( _filename, m_strImageFormat );
}

void KImageDocument::transformImage( const QWMatrix& matrix )
{
  QPixmap pix, newpix;

  pix.convertFromImage( m_image );
  newpix = pix.xForm( matrix );
  m_image = newpix.convertToImage();
  emit sigUpdateView();
  m_bModified = true;
  m_bEmpty = false;

  kdebug( KDEBUG_INFO, 0, "Image manipulated with matrix" );
}

void KImageDocument::setModified( bool _c )
{
  m_bModified = _c;
  if ( _c )
  {
    m_bEmpty = false;
  }
}

bool KImageDocument::isEmpty()
{
  return m_bEmpty;
}

float KImageDocument::printableWidth()
{
  return m_paperWidth - m_leftBorder - m_rightBorder;
}

float KImageDocument::printableHeight()
{
  return m_paperHeight - m_topBorder - m_bottomBorder;
}

float KImageDocument::paperHeight()
{
  return m_paperHeight;
}

float KImageDocument::paperWidth()
{
  return m_paperWidth;
}
  
float KImageDocument::leftBorder()
{
  return m_leftBorder;
}

float KImageDocument::rightBorder()
{
  return m_rightBorder;
}

float KImageDocument::topBorder()
{
  return m_topBorder;
}

float KImageDocument::bottomBorder()
{
  return m_bottomBorder;
}

KoOrientation KImageDocument::orientation()
{
  return m_orientation;
}

KoFormat KImageDocument::paperFormat()
{
  return m_paperFormat;
}

QString KImageDocument::headLeft( int _p, const char* _t )
{
  if( m_headLeft.isNull() )
  {
    return "";
  }
  return completeHeading( m_headLeft.data(), _p, _t );
}

QString KImageDocument::headRight( int _p, const char* _t )
{
  if( m_headRight.isNull() )
  {
    return "";
  }
  return completeHeading( m_headRight.data(), _p, _t );
}

QString KImageDocument::headMid( int _p, const char* _t )
{
  if( m_headMid.isNull() )
  {
    return "";
  }
  return completeHeading( m_headMid.data(), _p, _t );
}

QString KImageDocument::footLeft( int _p, const char* _t )
{
  if( m_footLeft.isNull() )
  {
    return "";
  }
  return completeHeading( m_footLeft.data(), _p, _t );
}

QString KImageDocument::footMid( int _p, const char* _t )
{
  if( m_footMid.isNull() )
  {
    return "";
  }
  return completeHeading( m_footMid.data(), _p, _t );
}

QString KImageDocument::footRight( int _p, const char* _t )
{
  if( m_footRight.isNull() )
  {
    return "";
  }
  return completeHeading( m_footRight.data(), _p, _t );
}

QString KImageDocument::headLeft()
{
  if( m_headLeft.isNull() )
  {
    return "";
  }
  return m_headLeft.data();
}

QString KImageDocument::headMid()
{
  if( m_headMid.isNull() )
  {
    return "";
  }
  return m_headMid.data();
}

QString KImageDocument::headRight()
{
  if( m_headRight.isNull() )
  {
    return "";
  }
  return m_headRight.data();
}

QString KImageDocument::footLeft()
{
  if( m_footLeft.isNull() )
  {
    return "";
  }
  return m_footLeft.data();
}

QString KImageDocument::footMid()
{
  if( m_footMid.isNull() )
  {
    return "";
  }
  return m_footMid.data();
}

QString KImageDocument::footRight()
{
  if( m_footRight.isNull() )
  {
    return "";
  }
  return m_footRight.data();
}

const QImage& KImageDocument::image()
{
  return m_image;
}

KImageDocument::~KImageDocument()
{
}
*/

#include "kimage_doc.moc"
