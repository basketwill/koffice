/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

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

#include "koVariable.h"
#include "koVariable.moc"
#include <koDocumentInfo.h>
#include <kotextparag.h>
#include <kozoomhandler.h>
#include <klocale.h>
#include <kdebug.h>
#include <kglobal.h>
#include <qdom.h>
#include <koUtils.h>
#include <koDocument.h>

QString KoVariableDateFormat::convert( const QDate & date ) const
{
    return KGlobal::locale()->formatDate( date, m_bShort );
}

QCString KoVariableDateFormat::key() const
{
    return QCString("DATE") + (m_bShort ? '1' : '0');
}

void KoVariableDateFormat::load( const QCString &key )
{
    QCString params( key.mid( 4 ) );
    if ( !params.isEmpty() )
        m_bShort = (params[0] == '1');
    // TODO else: use the last setting ?  (useful for the interactive case)
}

QString KoVariableTimeFormat::convert( const QTime & time ) const
{
    return KGlobal::locale()->formatTime( time );
}

QCString KoVariableTimeFormat::key() const
{
    return "TIME";
}

QString KoVariableStringFormat::convert( const QString & string ) const
{
    return string;
}

QCString KoVariableStringFormat::key() const
{
    return "STRING";
    // TODO prefix & suffix
}

QString KoVariableNumberFormat::convert( int value /*double ? QVariant ?*/ ) const
{
    return QString::number( value );
}

QCString KoVariableNumberFormat::key() const
{
    return "NUMBER";
}

/* for the prefix+suffix string format
    QString str;
    str.prepend( pre );
    str.append( post );
    return QString( str );
*/


KoVariableFormatCollection::KoVariableFormatCollection()
{
    m_dict.setAutoDelete( true );
}

KoVariableFormat * KoVariableFormatCollection::format( const QCString &key )
{
    KoVariableFormat *f = m_dict[ key.data() ];
    if (f)
        return f;
    else
        return createFormat( key );
}

KoVariableFormat * KoVariableFormatCollection::createFormat( const QCString &key )
{
    KoVariableFormat * format = 0L;
    // The first 4 chars identify the class
    QCString type = key.left(4);
    if ( type == "DATE" )
        format = new KoVariableDateFormat();
    else if ( type == "TIME" )
        format = new KoVariableTimeFormat();
    else if ( type == "NUMB" ) // this type of programming makes me numb ;)
        format = new KoVariableNumberFormat();
    else if ( type == "STRI" )
        format = new KoVariableStringFormat();

    if ( format )
    {
        format->load( key );
        m_dict.insert( format->key() /* not 'key', it could be incomplete */, format );
    }
    return format;
}

/******************************************************************/
/* Class:       KoVariableCollection                              */
/******************************************************************/
KoVariableCollection::KoVariableCollection()
{
}


void KoVariableCollection::registerVariable( KoVariable *var )
{
    if ( !var )
        return;
    variables.append( var );
}

void KoVariableCollection::unregisterVariable( KoVariable *var )
{
    variables.take( variables.findRef( var ) );
}

void KoVariableCollection::recalcVariables(int type)
{
    bool update = false;
    QPtrListIterator<KoVariable> it( variables );
    for ( ; it.current() ; ++it )
    {
        if ( it.current()->type() == type )
        {
            update = true;
            it.current()->recalc();
            Qt3::QTextParag * parag = it.current()->paragraph();
            if ( parag )
            {
                kdDebug() << "KoDoc::recalcVariables -> invalidating parag " << parag->paragId() << endl;
                parag->invalidate( 0 );
                parag->setChanged( true );
#if 0
                KWTextFrameSet * textfs = static_cast<KWTextDocument *>(it.current()->textDocument())->textFrameSet();
                if ( toRepaint.findRef( textfs ) == -1 )
                    toRepaint.append( textfs );
#endif
            }
        }
    }
    if(update)
        emit repaintVariable();
}


void KoVariableCollection::setVariableValue( const QString &name, const QString &value )
{
    varValues[ name ] = value;
}

QString KoVariableCollection::getVariableValue( const QString &name ) const
{
    if ( !varValues.contains( name ) )
        return i18n( "No value" );
    return varValues[ name ];
}

/******************************************************************/
/* Class: KoVariable                                              */
/******************************************************************/
KoVariable::KoVariable( KoTextDocument *textdoc, KoVariableFormat *varFormat, KoVariableCollection *_varColl)
    : KoTextCustomItem( textdoc )
{
    m_varColl=_varColl;
    m_varFormat = varFormat;
    m_varColl->registerVariable( this );
}

KoVariable::~KoVariable()
{
    //kdDebug() << "KoVariable::~KoVariable " << this << endl;
    m_varColl->unregisterVariable( this );
}

void KoVariable::resize()
{
    if ( m_deleted )
        return;
    QTextFormat *fmt = format();
    QString txt = text();
    width = 0;
    for ( int i = 0 ; i < (int)txt.length() ; ++i )
        width += fmt->width( txt, i );
    height = fmt->height();
    //kdDebug() << "Before KoVariable::resize text=" << txt << " width=" << width << endl;
}

void KoVariable::drawCustomItem( QPainter* p, int x, int y, int /*cx*/, int /*cy*/, int /*cw*/, int /*ch*/, const QColorGroup& cg, bool selected, int offset )
{
    KoTextFormat * f = static_cast<KoTextFormat *>(format());
    KoZoomHandler * zh = textDocument()->paintingZoomHandler();
    int bl, _y;
    KoTextParag * parag = static_cast<KoTextParag *>( paragraph() );
    //kdDebug() << "KoVariable::draw index=" << index() << " x=" << x << " y=" << y << endl;
    int h = parag->lineHeightOfChar( index(), &bl, &_y /*unused*/);

    h = zh->layoutUnitToPixelY( y, h );
    bl = zh->layoutUnitToPixelY( y, bl );

    p->save();
    p->setPen( QPen( f->color() ) );
    if ( f->textBackgroundColor().isValid() )
        p->fillRect( x, y, zh->layoutUnitToPixelX( width ), h, f->textBackgroundColor() );
    if ( selected )
    {
        p->setPen( QPen( cg.color( QColorGroup::HighlightedText ) ) );
        p->fillRect( x, y,  zh->layoutUnitToPixelX( width ), h, cg.color( QColorGroup::Highlight ) );

    }
#if 0
    else if ( parag->koTextDocument()->textFrameSet() &&
                parag->koTextDocument()->textFrameSet()->kWordDocument()->viewFormattingChars() && p->device()->devType() != QInternal::Printer )
    {
        p->setPen( QPen( cg.color( QColorGroup::Highlight ), 0, Qt::DotLine ) );
        p->drawRect( x, y, zh->layoutUnitToPixelX( width ), h );
    }
#endif
    //p->setFont( customItemFont ); // already done by the caller
    //kdDebug() << "KoVariable::draw bl=" << bl << << endl;
    p->drawText( x, y + bl + offset, text() );
    p->restore();
}

void KoVariable::save( QDomElement &formatElem )
{
    kdDebug() << "KoVariable::save" << endl;
    formatElem.setAttribute( "id", 4 ); // code for a variable
    QDomElement typeElem = formatElem.ownerDocument().createElement( "TYPE" );
    formatElem.appendChild( typeElem );
    typeElem.setAttribute( "type", static_cast<int>( type() ) );
    typeElem.setAttribute( "key", m_varFormat->key() );
}

void KoVariable::load( QDomElement & )
{
}

//static
KoVariable * KoVariable::createVariable( int type, int subtype, KoVariableFormatCollection * coll, KoVariableFormat *varFormat,KoTextDocument *textdoc, KoDocument * doc,KoVariableCollection *varColl )
{
    if ( varFormat == 0L )
    {
        // Get the default format for this variable (this method is only called in the interactive case, not when loading)
        switch ( type ) {
        case VT_DATE:
            varFormat = coll->format( "DATE" );
            break;
        case VT_TIME:
            varFormat = coll->format( "TIME" );
            break;
        case VT_PGNUM:
            varFormat = coll->format( "NUMBER" );
            break;
        case VT_FIELD:
            varFormat = coll->format( "STRING" );
            break;
        case VT_CUSTOM:
            varFormat = coll->format( "STRING" );
            break;
        case VT_SERIALLETTER:
            varFormat = coll->format( "STRING" );
            break;
        }
    }
    Q_ASSERT( varFormat );
    if ( varFormat == 0L ) // still 0 ? Impossible!
        return 0L ;

    KoVariable * var = 0L;
    switch ( type ) {
        case VT_DATE:
            var = new KoDateVariable( textdoc, subtype, varFormat,varColl );
            break;
        case VT_TIME:
            var = new KoTimeVariable( textdoc, subtype, varFormat, varColl );
            break;
        case VT_PGNUM:
            var = new KoPgNumVariable( textdoc, subtype, varFormat, varColl );
            break;
        case VT_FIELD:
            var = new KoFieldVariable( textdoc, subtype, varFormat,varColl,doc );
            break;
        case VT_CUSTOM:
            var = new KoCustomVariable( textdoc, QString::null, varFormat, varColl);
            break;
        case VT_SERIALLETTER:
            var = new KoSerialLetterVariable( textdoc, QString::null, varFormat ,varColl);
            break;
    }
    Q_ASSERT( var );
    return var;
}

/******************************************************************/
/* Class: KoDateVariable                                          */
/******************************************************************/
KoDateVariable::KoDateVariable( KoTextDocument *textdoc, int subtype, KoVariableFormat *_varFormat, KoVariableCollection *_varColl)
    : KoVariable( textdoc, _varFormat,_varColl ), m_subtype( subtype )
{
}

void KoDateVariable::recalc()
{
    if ( m_subtype == VST_DATE_CURRENT )
        m_date = QDate::currentDate();
    else
    {
        // Only if never set before (i.e. upon insertion)
        if ( m_date.isNull() )
            m_date = QDate::currentDate();
    }
    resize();
}

QString KoDateVariable::text()
{
    KoVariableDateFormat * format = dynamic_cast<KoVariableDateFormat *>( m_varFormat );
    Q_ASSERT( format );
    if ( format )
        return format->convert( m_date );
    // make gcc happy
    return QString::null;
}

void KoDateVariable::save( QDomElement& parentElem )
{
    KoVariable::save( parentElem );

    QDomElement elem = parentElem.ownerDocument().createElement( "DATE" );
    parentElem.appendChild( elem );
    elem.setAttribute( "year", m_date.year() );
    elem.setAttribute( "month", m_date.month() );
    elem.setAttribute( "day", m_date.day() );
    elem.setAttribute( "fix", m_subtype == VST_DATE_FIX ); // to be extended
}

void KoDateVariable::load( QDomElement& elem )
{
    KoVariable::load( elem );

    QDomElement e = elem.namedItem( "DATE" ).toElement();
    if (!e.isNull())
    {
        int y = e.attribute("year").toInt();
        int m = e.attribute("month").toInt();
        int d = e.attribute("day").toInt();
        bool fix = e.attribute("fix").toInt() == 1;
        if ( fix )
            m_date.setYMD( y, m, d );
        m_subtype = fix ? VST_DATE_FIX : VST_DATE_CURRENT;
    }
}

QStringList KoDateVariable::actionTexts()
{
    QStringList lst;
    lst << i18n( "Current date (fixed)" );
    lst << i18n( "Current date (variable)" );
    // TODO add date created, date printed, date last modified( BR #24242 )
    return lst;
}

/******************************************************************/
/* Class: KoTimeVariable                                          */
/******************************************************************/
KoTimeVariable::KoTimeVariable( KoTextDocument *textdoc, int subtype, KoVariableFormat *varFormat, KoVariableCollection *_varColl)
    : KoVariable( textdoc, varFormat,_varColl ), m_subtype( subtype )
{
}

void KoTimeVariable::recalc()
{
    if ( m_subtype == VST_TIME_CURRENT )
        m_time = QTime::currentTime();
    else
    {
        // Only if never set before (i.e. upon insertion)
        if ( m_time.isNull() )
            m_time = QTime::currentTime();
    }
    resize();
}

QString KoTimeVariable::text()
{
    KoVariableTimeFormat * format = dynamic_cast<KoVariableTimeFormat *>( m_varFormat );
    Q_ASSERT( format );
    if ( format )
        return format->convert( m_time );
    // make gcc happy
    return QString::null;
}

void KoTimeVariable::save( QDomElement& parentElem )
{
    KoVariable::save( parentElem );

    QDomElement elem = parentElem.ownerDocument().createElement( "TIME" );
    parentElem.appendChild( elem );
    elem.setAttribute( "hour", m_time.hour() );
    elem.setAttribute( "minute", m_time.minute() );
    elem.setAttribute( "second", m_time.second() );
    elem.setAttribute( "msecond", m_time.msec() );
    elem.setAttribute( "fix", m_subtype == VST_TIME_FIX );
}

void KoTimeVariable::load( QDomElement& elem )
{
    KoVariable::load( elem );

    QDomElement e = elem.namedItem( "TIME" ).toElement();
    if (!e.isNull())
    {
        int h = e.attribute("hour").toInt();
        int m = e.attribute("minute").toInt();
        int s = e.attribute("second").toInt();
        int ms = e.attribute("msecond").toInt();
        bool fix = static_cast<bool>( e.attribute("fix").toInt() );
        if ( fix )
            m_time.setHMS( h, m, s, ms );
        m_subtype = fix ? VST_TIME_FIX : VST_TIME_CURRENT;
    }
}

QStringList KoTimeVariable::actionTexts()
{
    QStringList lst;
    lst << i18n( "Current time (fixed)" );
    lst << i18n( "Current time (variable)" );
    // TODO add time created, time printed, time last modified( BR #24242 )
    return lst;
}


/******************************************************************/
/* Class: KoCustomVariable                                        */
/******************************************************************/
KoCustomVariable::KoCustomVariable( KoTextDocument *textdoc, const QString &name, KoVariableFormat *varFormat, KoVariableCollection *_varColl )
    : KoVariable( textdoc, varFormat,_varColl ), m_name( name )
{
}

void KoCustomVariable::save( QDomElement& parentElem )
{
    KoVariable::save( parentElem );
    QDomElement elem = parentElem.ownerDocument().createElement( "CUSTOM" );
    parentElem.appendChild( elem );
    elem.setAttribute( "name", correctQString( m_name ) );
    elem.setAttribute( "value", correctQString( value() ) );
}

void KoCustomVariable::load( QDomElement& elem )
{
    KoVariable::load( elem );
    QDomElement e = elem.namedItem( "CUSTOM" ).toElement();
    if (!e.isNull())
    {
        m_name = e.attribute( "name" );
        setValue( e.attribute( "value" ) );
    }
}

QString KoCustomVariable::value() const
{
    return m_varColl->getVariableValue( m_name );
}

void KoCustomVariable::setValue( const QString &v )
{
    m_varColl->setVariableValue( m_name, v );
}

QStringList KoCustomVariable::actionTexts()
{
    return QStringList( i18n( "Custom..." ) );
}

void KoCustomVariable::recalc()
{
    resize();
}

/******************************************************************/
/* Class: KoSerialLetterVariable                                  */
/******************************************************************/
KoSerialLetterVariable::KoSerialLetterVariable( KoTextDocument *textdoc, const QString &name, KoVariableFormat *varFormat,KoVariableCollection *_varColl )
    : KoVariable( textdoc, varFormat, _varColl ), m_name( name )
{
}

void KoSerialLetterVariable::save( QDomElement& parentElem )
{
    KoVariable::save( parentElem );
    QDomElement elem = parentElem.ownerDocument().createElement( "SERIALLETTER" );
    parentElem.appendChild( elem );
    elem.setAttribute( "name", correctQString( m_name ) );
}

void KoSerialLetterVariable::load( QDomElement& elem )
{
    KoVariable::load( elem );
    QDomElement e = elem.namedItem( "SERIALLETTER" ).toElement();
    if (!e.isNull())
        m_name = e.attribute( "name" );
}

QString KoSerialLetterVariable::value() const
{
    return QString();//m_doc->getSerialLetterDataBase()->getValue( m_name );
}

QString KoSerialLetterVariable::text()
{
    // ## should use a format maybe
    QString v = value();
    if ( v == name() )
        return "<" + v + ">";
    return v;
}

QStringList KoSerialLetterVariable::actionTexts()
{
    return QStringList( i18n( "&Serial Letter..." ) );
}

/******************************************************************/
/* Class: KoPgNumVariable                                         */
/******************************************************************/
KoPgNumVariable::KoPgNumVariable( KoTextDocument *textdoc, int subtype, KoVariableFormat *varFormat,KoVariableCollection *_varColl )
        : KoVariable( textdoc, varFormat,_varColl ), m_subtype( subtype ), m_pgNum( 0 )
{
}

void KoPgNumVariable::save( QDomElement& parentElem )
{
    KoVariable::save( parentElem );
    QDomElement pgNumElem = parentElem.ownerDocument().createElement( "PGNUM" );
    parentElem.appendChild( pgNumElem );
    pgNumElem.setAttribute( "subtype", m_subtype );
    pgNumElem.setAttribute( "value", m_pgNum );
}

void KoPgNumVariable::load( QDomElement& elem )
{
    KoVariable::load( elem );
    QDomElement pgNumElem = elem.namedItem( "PGNUM" ).toElement();
    if (!pgNumElem.isNull())
    {
        m_subtype = pgNumElem.attribute("subtype").toInt();
        m_pgNum = pgNumElem.attribute("value").toInt();
    }
}

void KoPgNumVariable::recalc()
{
    m_pgNum = 1;
    resize();
}

QString KoPgNumVariable::text()
{
    KoVariableNumberFormat * format = dynamic_cast<KoVariableNumberFormat *>( m_varFormat );
    Q_ASSERT( format );
    if ( format )
        return format->convert( m_pgNum );
    // make gcc happy
    return QString::null;
}

QStringList KoPgNumVariable::actionTexts()
{
    QStringList lst;
    lst << i18n( "Page Number" );
    lst << i18n( "Number Of Pages" );
    return lst;
}

/******************************************************************/
/* Class: KoFieldVariable                                         */
/******************************************************************/
KoFieldVariable::KoFieldVariable( KoTextDocument *textdoc, int subtype, KoVariableFormat *varFormat, KoVariableCollection *_varColl ,KoDocument *_doc )
    : KoVariable( textdoc, varFormat,_varColl ), m_subtype( subtype ), m_doc(_doc)
{
}

void KoFieldVariable::save( QDomElement& parentElem )
{
    //kdDebug() << "KoFieldVariable::save" << endl;
    KoVariable::save( parentElem );
    QDomElement elem = parentElem.ownerDocument().createElement( "FIELD" );
    parentElem.appendChild( elem );
    elem.setAttribute( "subtype", m_subtype );
    elem.setAttribute( "value", correctQString( m_value ) );
}

void KoFieldVariable::load( QDomElement& elem )
{
    KoVariable::load( elem );
    QDomElement e = elem.namedItem( "FIELD" ).toElement();
    if (!e.isNull())
    {
        m_subtype = e.attribute( "subtype" ).toInt();
        if ( m_subtype == VST_NONE )
            kdWarning() << "Field subtype of -1 found in the file !" << endl;
        m_value = e.attribute( "value" );
    } else
        kdWarning() << "FIELD element not found !" << endl;
}

void KoFieldVariable::recalc()
{
    switch( m_subtype ) {
        case VST_NONE:
            kdWarning() << "KoFieldVariable::recalc() called with m_subtype = VST_NONE !" << endl;
            break;
        case VST_FILENAME:
            m_value = m_doc->url().filename();
            break;
        case VST_DIRECTORYNAME:
            m_value = m_doc->url().directory();
            break;
        case VST_AUTHORNAME:
        case VST_EMAIL:
        case VST_COMPANYNAME:
        {
            KoDocumentInfo * info = m_doc->documentInfo();
            KoDocumentInfoAuthor * authorPage = static_cast<KoDocumentInfoAuthor *>(info->page( "author" ));
            if ( !authorPage )
                kdWarning() << "Author information not found in documentInfo !" << endl;
            else
            {
                if ( m_subtype == VST_AUTHORNAME )
                    m_value = authorPage->fullName();
                else if ( m_subtype == VST_EMAIL )
                    m_value = authorPage->email();
                else if ( m_subtype == VST_COMPANYNAME )
                    m_value = authorPage->company();
            }
        }
        break;
        case VST_TITLE:
        case VST_ABSTRACT:
        {
            KoDocumentInfo * info = m_doc->documentInfo();
            KoDocumentInfoAbout * aboutPage = static_cast<KoDocumentInfoAbout *>(info->page( "about" ));
            if ( !aboutPage )
                kdWarning() << "'About' page not found in documentInfo !" << endl;
            else
            {
                if ( m_subtype == VST_TITLE )
                    m_value = aboutPage->title();
                else
                    m_value = aboutPage->abstract();
            }
        }
        break;
    }

    if ( m_value.isEmpty() )
        m_value = i18n("<None>");
    resize();
}

QStringList KoFieldVariable::actionTexts()
{
    QStringList lst;
    lst << i18n( "File Name" );
    lst << i18n( "Directory Name" ); // is "Name" necessary ?
    lst << i18n( "Author Name" ); // is "Name" necessary ?
    lst << i18n( "Email" );
    lst << i18n( "Company Name" ); // is "Name" necessary ?
    lst << QString::null; //5
    lst << QString::null; //6
    lst << QString::null; //7
    lst << QString::null; //8
    lst << QString::null; //9
    lst << i18n( "Document Title" );
    lst << i18n( "Document Abstract" );
    return lst;
}

