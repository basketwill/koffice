/* This file is part of the KDE project
   Copyright (C) 2002 Laurent Montel <lmontel@mandrakesoft.com>

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

#include "ooimpressimport.h"

#include <math.h>

#include <qregexp.h>

#include <kdebug.h>
#include <koUnit.h>
#include <koDocumentInfo.h>
#include <koDocument.h>

#include <kgenericfactory.h>
#include <koFilterChain.h>
#include <koGlobal.h>

typedef KGenericFactory<OoImpressImport, KoFilter> OoImpressImportFactory;
K_EXPORT_COMPONENT_FACTORY( libooimpressimport, OoImpressImportFactory( "ooimpressimport" ) );


OoImpressImport::OoImpressImport( KoFilter *, const char *, const QStringList & )
    : KoFilter(),
      m_styles( 23, true )
{
    m_styles.setAutoDelete( true );
}

OoImpressImport::~OoImpressImport()
{
}

KoFilter::ConversionStatus OoImpressImport::convert( QCString const & from, QCString const & to )
{
    kdDebug() << "Entering Ooimpress Import filter: " << from << " - " << to << endl;

    if ( from != "application/vnd.sun.xml.impress" || to != "application/x-kpresenter" )
    {
        kdWarning() << "Invalid mimetypes " << from << " " << to << endl;
        return KoFilter::NotImplemented;
    }

    KoFilter::ConversionStatus preStatus = openFile();

    if ( preStatus != KoFilter::OK )
        return preStatus;

    QDomDocument docinfo;
    createDocumentInfo( docinfo );

    // store document info
    KoStoreDevice* out = m_chain->storageFile( "documentinfo.xml", KoStore::Write );
    if( out )
    {
        QCString info = docinfo.toCString();
        //kdDebug() << " info :" << info << endl;
        // WARNING: we cannot use KoStore::write(const QByteArray&) because it gives an extra NULL character at the end.
        out->writeBlock( info , info.length() );
    }

    QDomDocument doccontent;
    createDocumentContent( doccontent );

    // store document content
    out = m_chain->storageFile( "maindoc.xml", KoStore::Write );
    if( out )
    {
        QCString content = doccontent.toCString();
        //kdDebug() << " content :" << content << endl;
        out->writeBlock( content , content.length() );
    }

    kdDebug() << "######################## OoImpressImport::convert done ####################" << endl;
    return KoFilter::OK;
}

KoFilter::ConversionStatus OoImpressImport::openFile()
{
  KoStore * store = KoStore::createStore( m_chain->inputFile(), KoStore::Read);

  if ( !store )
  {
    kdWarning() << "Couldn't open the requested file." << endl;
    return KoFilter::FileNotFound;
  }

  if ( !store->open( "content.xml" ) )
  {
    kdWarning() << "This file doesn't seem to be a valid OoImpress file" << endl;
    delete store;
    return KoFilter::WrongFormat;
  }

  QDomDocument styles;
  QCString totalString;
  char tempData[1024];

  Q_LONG size = store->read( &tempData[0], 1023 );
  while ( size > 0 )
  {
    QCString tempString( tempData, size + 1);
    totalString += tempString;

    size = store->read( &tempData[0], 1023 );
  }

  m_content.setContent( totalString );
  totalString = "";
  store->close();
  //kdDebug() << "m_content.toCString() :" << m_content.toCString() << endl;
  kdDebug() << "File containing content loaded " << endl;

  if ( store->open( "styles.xml" ) )
  {
    size = store->read( &tempData[0], 1023 );
    while ( size > 0 )
    {
      QCString tempString( tempData, size + 1);
      totalString += tempString;

      size = store->read( &tempData[0], 1023 );
    }

    styles.setContent( totalString );
    totalString = "";
    store->close();
    //kdDebug() << "styles.toCString() :" << styles.toCString() << endl;
    kdDebug() << "File containing styles loaded" << endl;
  }
  else
    kdWarning() << "Style definitions do not exist!" << endl;

  if ( store->open( "meta.xml" ) )
  {
    size = store->read( &tempData[0], 1023 );
    while ( size > 0 )
    {
      QCString tempString( tempData, size + 1);
      totalString += tempString;

      size = store->read( &tempData[0], 1023 );
    }

    m_meta.setContent( totalString );
    totalString = "";
    store->close();
    kdDebug() << "File containing meta definitions loaded" << endl;
  }
  else
    kdWarning() << "Meta definitions do not exist!" << endl;

  if ( store->open( "settings.xml" ) )
  {
    size = store->read( &tempData[0], 1023 );
    while ( size > 0 )
    {
      QCString tempString( tempData, size + 1);
      totalString += tempString;

      size = store->read( &tempData[0], 1023 );
    }

    m_settings.setContent( totalString );
    totalString = "";
    store->close();
    kdDebug() << "File containing settings loaded" << endl;
  }
  else
    kdWarning() << "Settings do not exist!" << endl;

  delete store;

  emit sigProgress( 10 );
  createStyleMap( styles );

  return KoFilter::OK;
}

void OoImpressImport::createDocumentInfo( QDomDocument &docinfo )
{
    docinfo.appendChild( docinfo.createProcessingInstruction( "xml","version=\"1.0\" encoding=\"UTF-8\"" ) );
    QDomDocument doc = KoDocument::createDomDocument( "document-info" /*DTD name*/, "document-info" /*tag name*/, "1.1" );

    QDomNode meta   = m_meta.namedItem( "office:document-meta" );
    QDomNode office = meta.namedItem( "office:meta" );

    if ( office.isNull() )
        return;
    QDomElement elementDocInfo  = doc.documentElement();

    QDomElement e = office.namedItem( "dc:creator" ).toElement();
    if ( !e.isNull() && !e.text().isEmpty() )
    {
        QDomElement author = doc.createElement( "author" );
        QDomElement t = doc.createElement( "full-name" );
        author.appendChild( t );
        t.appendChild( doc.createTextNode( e.text() ) );
        elementDocInfo.appendChild( author);
    }

    e = office.namedItem( "dc:title" ).toElement();
    if ( !e.isNull() && !e.text().isEmpty() )
    {
        QDomElement about = doc.createElement( "about" );
        QDomElement title = doc.createElement( "title" );
        about.appendChild( title );
        title.appendChild( doc.createTextNode( e.text() ) );
        elementDocInfo.appendChild( about );
    }
#if 0
    e = office.namedItem( "dc:description" ).toElement();
    if ( !e.isNull() && !e.text().isEmpty() )
    {

    }
#endif
    docinfo.appendChild(doc);

    //kdDebug() << " meta-info :" << m_meta.toCString() << endl;
}

void OoImpressImport::createDocumentContent( QDomDocument &doccontent )
{
    QDomDocument doc = KoDocument::createDomDocument( "kpresenter", "DOC", "1.2" );
    QDomElement docElement = doc.documentElement();
    docElement.setAttribute( "editor", "KPresenter" );
    docElement.setAttribute( "mime", "application/x-kpresenter" );
    docElement.setAttribute( "syntaxVersion", "2" );

    QDomElement content = m_content.documentElement();

    // content.xml contains some automatic-styles that we need to store
    QDomNode automaticStyles = content.namedItem( "office:automatic-styles" );
    if ( !automaticStyles.isNull() )
        insertStyles( automaticStyles.toElement() );

    QDomNode body = content.namedItem( "office:body" );
    if ( body.isNull() )
        return;

    // it seems that ooimpress has different paper-settings for every slide.
    // we take the settings of the first slide for the whole document.
    QDomNode drawPage = body.namedItem( "draw:page" );
    if ( drawPage.isNull() ) // no slides? give up.
        return;

    QDomElement dp = drawPage.toElement();
    QDomElement *master = m_styles[dp.attribute( "draw:master-page-name" )];
    QDomElement *style = m_styles[master->attribute( "style:page-master-name" )];
    QDomElement properties = style->namedItem( "style:properties" ).toElement();

    double pageHeight;
    QDomElement paperElement = doc.createElement( "PAPER" );
    if ( properties.isNull() )
    {
        paperElement.setAttribute( "ptWidth", CM_TO_POINT(28) );
        paperElement.setAttribute( "ptHeight", CM_TO_POINT(21) );
        paperElement.setAttribute( "unit", 0 );
        paperElement.setAttribute( "format", 5 );
        paperElement.setAttribute( "tabStopValue", 42.5198 );
        paperElement.setAttribute( "orientation", 0 );
        pageHeight = 21;

        QDomElement paperBorderElement = doc.createElement( "PAPERBORDERS" );
        paperBorderElement.setAttribute( "ptRight", 0 );
        paperBorderElement.setAttribute( "ptBottom", 0 );
        paperBorderElement.setAttribute( "ptLeft", 0 );
        paperBorderElement.setAttribute( "ptTop", 0 );
        paperElement.appendChild( paperBorderElement );
    }
    else
    {
        paperElement.setAttribute( "ptWidth", toPoint(properties.attribute( "fo:page-width" ) ) );
        paperElement.setAttribute( "ptHeight", toPoint(properties.attribute( "fo:page-height" ) ) );
//         paperElement.setAttribute( "unit", 0 );
//         paperElement.setAttribute( "format", 5 );
//         paperElement.setAttribute( "tabStopValue", 42.5198 );
//         paperElement.setAttribute( "orientation", 0 );
        pageHeight = toPoint( properties.attribute( "fo:page-height" ) );

        QDomElement paperBorderElement = doc.createElement( "PAPERBORDERS" );
        paperBorderElement.setAttribute( "ptRight", toPoint( properties.attribute( "fo:margin-right" ) ) );
        paperBorderElement.setAttribute( "ptBottom", toPoint( properties.attribute( "fo:margin-bottom" ) ) );
        paperBorderElement.setAttribute( "ptLeft", toPoint( properties.attribute( "fo:page-left" ) ) );
        paperBorderElement.setAttribute( "ptTop", toPoint( properties.attribute( "fo:page-top" ) ) );
        paperElement.appendChild( paperBorderElement );
    }

    QDomElement objectElement = doc.createElement( "OBJECTS" );
    QDomElement pageTitleElement = doc.createElement( "PAGETITLES" );

    // parse all pages
    for ( drawPage = body.firstChild(); !drawPage.isNull(); drawPage = drawPage.nextSibling() )
    {
        dp = drawPage.toElement();

        // set the pagetitle
        QDomElement titleElement = doc.createElement( "Title" );
        titleElement.setAttribute( "title", dp.attribute( "draw:name" ) );
        pageTitleElement.appendChild( titleElement );

        double offset = ( dp.attribute( "draw:id" ).toInt() - 1 ) * pageHeight;

        // parse all objects
        for ( QDomNode object = drawPage.firstChild(); !object.isNull(); object = object.nextSibling() )
        {
            QDomElement o = object.toElement();
            QString name = o.tagName();

            QDomElement e;
            if ( name == "draw:text-box" ) // textbox
            {
                storeObjectStyles( o );
                e = doc.createElement( "OBJECT" );
                e.setAttribute( "type", 4 );
                append2DGeometry( doc, e, o, offset );
                appendPen( doc, e );
                appendBrush( doc, e );
                appendRounding( doc, e, o );
                appendShadow( doc, e );
                e.appendChild( parseTextBox( doc, o ) );
            }
            else if ( name == "draw:rect" ) // rectangle
            {
                storeObjectStyles( o );
                e = doc.createElement( "OBJECT" );
                e.setAttribute( "type", 2 );
                append2DGeometry( doc, e, o, offset );
                appendPen( doc, e );
                appendBrush( doc, e );
                appendRounding( doc, e, o );
                appendShadow( doc, e );
            }
            else if ( name == "draw:circle" || name == "draw:ellipse" ) // circle or ellipse
            {
                storeObjectStyles( o );
                e = doc.createElement( "OBJECT" );
                e.setAttribute( "type", 3 );
                append2DGeometry( doc, e, o, offset );
                appendPen( doc, e );
                appendBrush( doc, e );
                appendShadow( doc, e );
            }
            else if ( name == "draw:line" ) // line
            {
                storeObjectStyles( o );
                e = doc.createElement( "OBJECT" );
                e.setAttribute( "type", 1 );
                appendLineGeometry( doc, e, o, offset );
                appendPen( doc, e );
                appendBrush( doc, e );
                appendShadow( doc, e );
                appendLineEnds( doc, e );
            }
            else
            {
                kdDebug() << "Unsupported object '" << name << "'" << endl;
                continue;
            }

            objectElement.appendChild( e );
        }
    }

    docElement.appendChild( paperElement );
    docElement.appendChild( pageTitleElement );
    docElement.appendChild( objectElement );
    doccontent.appendChild( doc );
}

void OoImpressImport::append2DGeometry( QDomDocument& doc, QDomElement& e, const QDomElement& object, int offset )
{
    QDomElement orig = doc.createElement( "ORIG" );
    orig.setAttribute( "x", toPoint( object.attribute( "svg:x" ) ) );
    orig.setAttribute( "y", toPoint( object.attribute( "svg:y" ) ) + offset );
    e.appendChild( orig );

    QDomElement size = doc.createElement( "SIZE" );
    size.setAttribute( "width", toPoint( object.attribute( "svg:width" ) ) );
    size.setAttribute( "height", toPoint( object.attribute( "svg:height" ) ) );
    e.appendChild( size );
}

void OoImpressImport::appendLineGeometry( QDomDocument& doc, QDomElement& e, const QDomElement& object, int offset )
{
    double x1 = toPoint( object.attribute( "svg:x1" ) );
    double y1 = toPoint( object.attribute( "svg:y1" ) );
    double x2 = toPoint( object.attribute( "svg:x2" ) );
    double y2 = toPoint( object.attribute( "svg:y2" ) );

    double x = QMIN( x1, x2 );
    double y = QMIN( y1, y2 );

    QDomElement orig = doc.createElement( "ORIG" );
    orig.setAttribute( "x", x );
    orig.setAttribute( "y", y + offset );
    e.appendChild( orig );

    QDomElement size = doc.createElement( "SIZE" );
    size.setAttribute( "width", fabs( x1 - x2 ) );
    size.setAttribute( "height", fabs( y1 - y2 ) );
    e.appendChild( size );

    QDomElement linetype = doc.createElement( "LINETYPE" );
    if ( ( x1 < x2 && y1 < y2 ) || ( x1 > x2 && y1 > y2 ) )
        linetype.setAttribute( "value", 2 );
    else
        linetype.setAttribute( "value", 3 );

    e.appendChild( linetype );
}

void OoImpressImport::appendPen( QDomDocument& doc, QDomElement& e )
{
    QDomElement pen = doc.createElement( "PEN" );
    if ( m_styleStack.hasAttribute( "draw:stroke" ) )
    {
        if ( m_styleStack.attribute( "draw:stroke" ) == "solid" ) // TODO check for other styles
            pen.setAttribute( "style", 1 );
        if ( m_styleStack.hasAttribute( "svg:stroke-width" ) )
            pen.setAttribute( "width", (int) toPoint( m_styleStack.attribute( "svg:stroke-width" ) ) );
        if ( m_styleStack.hasAttribute( "svg:stroke-color" ) )
            pen.setAttribute( "color", m_styleStack.attribute( "svg:stroke-color" ) );
        e.appendChild( pen );
    }
    else
        pen.setAttribute( "style", 0 ); // to avoid a line around textobjects
}

void OoImpressImport::appendBrush( QDomDocument& doc, QDomElement& e )
{
    if ( m_styleStack.hasAttribute( "draw:fill" ) )
    {
        QDomElement brush = doc.createElement( "BRUSH" );
        if ( m_styleStack.attribute( "draw:fill" ) == "solid" ) // TODO check for other styles
            brush.setAttribute( "style", 1 );
        if ( m_styleStack.hasAttribute( "draw:fill-color" ) )
            brush.setAttribute( "color", m_styleStack.attribute( "draw:fill-color" ) );
        e.appendChild( brush );
    }
}

void OoImpressImport::appendRounding( QDomDocument& doc, QDomElement& e, const QDomElement& object )
{
    if ( object.hasAttribute( "draw:corner-radius" ) )
    {
        // kpresenter uses percent, ooimpress uses cm ... hmm?
        QDomElement rounding = doc.createElement( "RNDS" );
        rounding.setAttribute( "x", (int) toPoint( object.attribute( "draw:corner-radius" ) ) );
        rounding.setAttribute( "y", (int) toPoint( object.attribute( "draw:corner-radius" ) ) );
        e.appendChild( rounding );
    }
}

void OoImpressImport::appendShadow( QDomDocument& doc, QDomElement& e )
{
    // Note that ooimpress makes a difference between shadowed text and
    // a shadowed object while kpresenter only knows the attribute 'shadow'.
    // This means that a shadowed textobject in kpresenter will always show
    // a shadowed text but no shadow for the object itself.

    // make sure this is a textobject or textspan
    if ( !e.hasAttribute( "type" ) ||
         ( e.hasAttribute( "type" ) && e.attribute( "type" ) == "4" ) )
    {
        if ( m_styleStack.hasAttribute( "fo:text-shadow" ) &&
             m_styleStack.attribute( "fo:text-shadow" ) != "none" )
        {
            // use the shadow attribute to indicate a text-shadow
            QDomElement shadow = doc.createElement( "SHADOW" );
            QString distance = m_styleStack.attribute( "fo:text-shadow" );
            distance.truncate( distance.find( ' ' ) );
            shadow.setAttribute( "distance", toPoint( distance ) );
            shadow.setAttribute( "direction", 5 );
            shadow.setAttribute( "color", "#a0a0a0" );
            e.appendChild( shadow );
        }
    }
    else if ( m_styleStack.hasAttribute( "draw:shadow" ) &&
              m_styleStack.attribute( "draw:shadow" ) == "visible" )
    {
        // use the shadow attribute to indicate an object-shadow
        QDomElement shadow = doc.createElement( "SHADOW" );
        double x = toPoint( m_styleStack.attribute( "draw:shadow-offset-x" ) );
        double y = toPoint( m_styleStack.attribute( "draw:shadow-offset-y" ) );

        if ( x < 0 && y < 0 )
        {
            shadow.setAttribute( "direction", 1 );
            shadow.setAttribute( "distance", (int) fabs ( x ) );
        }
        else if ( x == 0 && y < 0 )
        {
            shadow.setAttribute( "direction", 2 );
            shadow.setAttribute( "distance", (int) fabs ( y ) );
        }
        else if ( x > 0 && y < 0 )
        {
            shadow.setAttribute( "direction", 3 );
            shadow.setAttribute( "distance", (int) fabs ( x ) );
        }
        else if ( x > 0 && y == 0 )
        {
            shadow.setAttribute( "direction", 4 );
            shadow.setAttribute( "distance", (int) fabs ( x ) );
        }
        else if ( x > 0 && y > 0 )
        {
            shadow.setAttribute( "direction", 5 );
            shadow.setAttribute( "distance", (int) fabs ( x ) );
        }
        else if ( x == 0 && y > 0 )
        {
            shadow.setAttribute( "direction", 6 );
            shadow.setAttribute( "distance", (int) fabs ( y ) );
        }
        else if ( x < 0 && y > 0 )
        {
            shadow.setAttribute( "direction", 7 );
            shadow.setAttribute( "distance", (int) fabs ( x ) );
        }
        else if ( x < 0 && y == 0 )
        {
            shadow.setAttribute( "direction", 8 );
            shadow.setAttribute( "distance", (int) fabs ( x ) );
        }

        if ( m_styleStack.hasAttribute ( "draw:shadow-color" ) )
            shadow.setAttribute( "color", m_styleStack.attribute( "draw:shadow-color" ) );

        e.appendChild( shadow );
    }
}

void OoImpressImport::appendLineEnds( QDomDocument& doc, QDomElement& e )
{
}

double OoImpressImport::toPoint( QString value )
{
    value.simplifyWhiteSpace();
    value.remove( ' ' );

    int index = value.find( QRegExp( "[a-z]{1,2}$" ), -1 );
    if ( index == -1 )
        return 0;

    QString unit = value.mid( index - 1 );
    value.truncate ( index - 1 );

    if ( unit == "cm" )
        return CM_TO_POINT( value.toDouble() );
    else if ( unit == "mm" )
        return MM_TO_POINT( value.toDouble() );
    else if ( unit == "pt" )
        return value.toDouble();
    else
        return 0;
}

QDomElement OoImpressImport::parseTextBox( QDomDocument& doc, const QDomElement& textBox )
{
    QDomElement textObjectElement = doc.createElement( "TEXTOBJ" );
    //lukas: TODO the text box can have a style as well (presentation:style-name)!
    //percy: this should be fixed with the new StyleStack

    for ( QDomNode text = textBox.firstChild(); !text.isNull(); text = text.nextSibling() )
    {
        QDomElement t = text.toElement();
        QString name = t.tagName();

        QDomElement e;
        if ( name == "text:p" ) // text paragraph
            e = parseParagraph( doc, t );
        else if ( name == "text:unordered-list" || name == "text:ordered-list" ) // listitem
        {
            e = parseList( doc, t );
        }
        else
        {
            kdDebug() << "Unsupported texttype '" << name << "'" << endl;
            continue;
        }

        textObjectElement.appendChild( e );
        m_styleStack.clearMark( 0 ); // remove the styles added by the child-object
    }

    return textObjectElement;
}

QDomElement OoImpressImport::parseList( QDomDocument& doc, const QDomElement& list )
{
    // take care of nested lists
    //kdDebug() << k_funcinfo << "parsing list"<< endl;
    int indentation = 0;
    QDomElement e;
    for ( QDomNode n = list.firstChild(); !n.isNull(); n = n.firstChild() )
    {
        e = n.toElement();
        if ( e.tagName() == "text:unordered-list" || e.tagName() == "text:ordered-list" )
        {
            indentation += 10;
            // parse the list-properties
            fillStyleStack( e );
        }
        if ( e.tagName() == "text:p" )
            break;
    }

    QDomElement p = parseParagraph( doc, e );
    if (indentation != 0)
    {
        QDomElement indent = doc.createElement( "INDENTS" );
        indent.setAttribute( "left", MM_TO_POINT( indentation ) ); //lukas: is MM always correct?
        // percy: MM is correct as I count the number of indentations and take 10mm for every
        // indentation-level. But we could use the values from the corresponding list-style instead.
        // See styles L1, L2, L3...
        p.appendChild( indent );
    }

    QDomElement counter = doc.createElement( "COUNTER" ); // TODO when styles are done
    counter.setAttribute( "numberingtype", 0 );
    counter.setAttribute( "type", 10 );
    counter.setAttribute( "depth", 0 );
    p.appendChild( counter );

    return p;
}

QDomElement OoImpressImport::parseParagraph( QDomDocument& doc, const QDomElement& paragraph )
{
    QDomElement p = doc.createElement( "P" );

    // parse the paragraph-properties
    fillStyleStack( paragraph );

    if ( m_styleStack.hasAttribute( "fo:text-align" ) )
    {
        if ( m_styleStack.attribute( "fo:text-align" ) == "center" )
            p.setAttribute( "align", 4 );
        else if ( m_styleStack.attribute( "fo:text-align" ) == "justify" )
            p.setAttribute( "align", 8 );
        else if ( m_styleStack.attribute( "fo:text-align" ) == "start" )
            p.setAttribute( "align", 0 );
        else if ( m_styleStack.attribute( "fo:text-align" ) == "end" )
            p.setAttribute( "align", 2 );
    }
    else
        p.setAttribute( "align", 0 ); // use left aligned as default

    // parse every childnode of the paragraph
    for( QDomNode n = paragraph.firstChild(); !n.isNull(); n = n.nextSibling() )
    {
        QString textData;
        QDomText t = n.toText();
        if ( t.isNull() ) // no textnode, so maybe it's a text:span
        {
            QDomElement ts = n.toElement();
            if ( ts.tagName() != "text:span" ) // TODO: are there any other possible
                continue;                      // elements or even nested test:spans?

            m_styleStack.setMark( 1 ); // we have to remove the spans styles
            fillStyleStack( ts.toElement() );
            textData = ts.text();
        }
        else
            textData = t.data();

        QDomElement text = doc.createElement( "TEXT" );
        text.appendChild( doc.createTextNode( textData ) );

        //kdDebug() << k_funcinfo << "Para text is: " << paragraph.text() << endl;

        // parse the text-properties
        if ( m_styleStack.hasAttribute( "fo:color" ) )
            text.setAttribute( "color", m_styleStack.attribute( "fo:color" ) );
        if ( m_styleStack.hasAttribute( "fo:font-family" ) )
            text.setAttribute( "family", m_styleStack.attribute( "fo:font-family" ).remove( "'" ) );
        if ( m_styleStack.hasAttribute( "fo:font-size" ) )
            text.setAttribute( "pointSize", toPoint( m_styleStack.attribute( "fo:font-size" ) ) );
        if ( m_styleStack.hasAttribute( "fo:font-weight" ) )
            if ( m_styleStack.attribute( "fo:font-weight" ) == "bold" )
                text.setAttribute( "bold", 1 );
        if ( m_styleStack.hasAttribute( "fo:font-style" ) )
            if ( m_styleStack.attribute( "fo:font-style" ) == "italic" )
                text.setAttribute( "italic", 1 );
        if ( m_styleStack.hasAttribute( "style:text-underline" ) )
        {
            if ( m_styleStack.attribute( "style:text-underline" ) == "single" )
            {
                text.setAttribute( "underline", 1 );
                text.setAttribute( "underlinestyleline", "solid" );  //lukas: TODO support more underline styles
            }
        }

        appendShadow( doc, p ); // this is necessary to take care of shadowed paragraphs

        if ( n.toElement().tagName() == "text:span" )
            m_styleStack.clearMark( 1 ); // current node is a text:span, remove its style from the stack

        p.appendChild( text );
    }

    return p;
}

void OoImpressImport::createStyleMap( QDomDocument &docstyles )
{
  QDomElement styles = docstyles.documentElement();
  if ( styles.isNull() )
      return;

  QDomNode fixedStyles = styles.namedItem( "office:styles" );
  if ( !fixedStyles.isNull() )
      insertStyles( fixedStyles.toElement() );

  QDomNode automaticStyles = styles.namedItem( "office:automatic-styles" );
  if ( !automaticStyles.isNull() )
      insertStyles( automaticStyles.toElement() );

  QDomNode masterStyles = styles.namedItem( "office:master-styles" );
  if ( !masterStyles.isNull() )
      insertStyles( masterStyles.toElement() );
}

void OoImpressImport::insertStyles( const QDomElement& styles )
{
    for ( QDomNode n = styles.firstChild(); !n.isNull(); n = n.nextSibling() )
    {
        QDomElement e = n.toElement();

        if ( !e.hasAttribute( "style:name" ) )
            continue;

        QString name = e.attribute( "style:name" );
        m_styles.insert( name, new QDomElement( e ) );
        //kdDebug() << "Style: '" << name << "' loaded " << endl;
    }
}

void OoImpressImport::fillStyleStack( const QDomElement& object )
{
    // find all styles associated with an object and push them on the stack
    if ( object.hasAttribute( "presentation:style-name" ) )
    {
        QDomElement *style = m_styles[object.attribute( "presentation:style-name" )];
        if ( style->hasAttribute( "style:parent-style-name" ) )
            m_styleStack.push( m_styles[style->attribute( "style:parent-style-name" )] );
        m_styleStack.push( style );
    }
    if ( object.hasAttribute( "draw:style-name" ) )
    {
        QDomElement *style = m_styles[object.attribute( "draw:style-name" )];
        if ( style->hasAttribute( "style:parent-style-name" ) )
            m_styleStack.push( m_styles[style->attribute( "style:parent-style-name" )] );
        m_styleStack.push( style );
    }
    if ( object.hasAttribute( "draw:text-style-name" ) )
    {
        QDomElement *style = m_styles[object.attribute( "draw:text-style-name" )];
        if ( style->hasAttribute( "style:parent-style-name" ) )
            m_styleStack.push( m_styles[style->attribute( "style:parent-style-name" )] );
        m_styleStack.push( style );
    }
    if ( object.hasAttribute( "text:style-name" ) )
    {
        QDomElement *style = m_styles[object.attribute( "text:style-name" )];
        if ( style->hasAttribute( "style:parent-style-name" ) )
            m_styleStack.push( m_styles[style->attribute( "style:parent-style-name" )] );
        m_styleStack.push( style );
    }
}

void OoImpressImport::storeObjectStyles( const QDomElement& object )
{
    m_styleStack.clear();
    fillStyleStack( object );
    m_styleStack.setMark( 0 );
}

StyleStack::StyleStack()
    : m_marks( 5 )
{
    m_marks.fill( 0 );
    m_stack.setAutoDelete( false );
}

StyleStack::~StyleStack()
{
}

void StyleStack::clear()
{
    m_marks.fill( 0 );
    m_stack.clear();
}

void StyleStack::clearMark( uint mark )
{
    if ( mark > m_marks.size() - 1 )
        m_marks.resize( mark );
    for ( uint index = m_stack.count() - 1; index >= m_marks[mark]; --index )
        m_stack.remove( index );
}

void StyleStack::setMark( uint mark )
{
    if ( mark > m_marks.size() - 1 )
        m_marks.resize( mark );
    m_marks[mark] = m_stack.count();
}

void StyleStack::pop()
{
    m_stack.removeLast();
}

void StyleStack::push( const QDomElement* style )
{
    m_stack.append( style );
}

bool StyleStack::hasAttribute( const QString& name )
{
    // TODO: has to be fixed for complex styles like list-styles
    for ( QDomElement *style = m_stack.last(); style; style = m_stack.prev() )
    {
        QDomElement properties = style->namedItem( "style:properties" ).toElement();
        if ( properties.hasAttribute( name ) )
            return true;
    }

    return false;
}

QString StyleStack::attribute( const QString& name )
{
    // TODO: has to be fixed for complex styles like list-styles
    for ( QDomElement *style = m_stack.last(); style; style = m_stack.prev() )
    {
        QDomElement properties = style->namedItem( "style:properties" ).toElement();
        if ( properties.hasAttribute( name ) )
            return properties.attribute( name );
    }

    return QString::null;
}

#include <ooimpressimport.moc>
