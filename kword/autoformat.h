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

#ifndef kwautoformat_h
#define kwautoformat_h

#include <qstring.h>
#include <qcolor.h>
#include <qmap.h>
#include <qvaluelist.h>

#include "format.h"

class KWordDocument;
class KWFormatContext;
class KWParag;
class KWString;
struct KWChar;

/******************************************************************/
/* Class: KWAutoFormatEntry					  */
/******************************************************************/

class KWAutoFormatEntry
{
public:
    KWAutoFormatEntry();

    void setFind( const QString &str );
    void setReplace( const QString &str );
    void setCheckFamily( bool b );
    void setCheckColor( bool b );
    void setCheckSize( bool b );
    void setCheckBold( bool b );
    void setCheckItalic( bool b );
    void setCheckUnderline( bool b );
    void setCheckVertAlign( bool b );
    void setFamily( const QString &str );
    void setColor( const QColor &c );
    void setSize( int size );
    void setBold( bool b );
    void setItalic( bool b );
    void setUnderline( bool b );
    void setVertAlign( KWFormat::VertAlign va );


    QString getFind();
    QString getReplace();
    bool getCheckFamily();
    bool getCheckColor();
    bool getCheckSize();
    bool getCheckBold();
    bool getCheckItalic();
    bool getCheckUnderline();
    bool getCheckVertAlign();
    QString getFamily();
    QColor getColor();
    int getSize();
    bool getBold();
    bool getItalic();
    bool getUnderline();
    bool getVertAlign();

    bool isValid()
    { return !find.isEmpty() && !replace.isEmpty(); }

protected:
    QString find, replace;
    bool checkFamily, checkColor, checkSize, checkBold, checkItalic, checkUnderline, checkVertAlign;
    QString family;
    QColor color;
    int size;
    bool bold, italic, underline;
    KWFormat::VertAlign vertAlign;

};

/******************************************************************/
/* Class: KWAutoFormat						  */
/******************************************************************/

class KWAutoFormat
{
public:
    struct TypographicQuotes
    {
	TypographicQuotes() : begin( ( char )'�' ), end( ( char )'�' ), replace( FALSE )
	{}
	TypographicQuotes( const TypographicQuotes &t ) {
	    begin = t.begin;
	    end = t.end;
	    replace = t.replace;
	}
	TypographicQuotes &operator=( const TypographicQuotes &t ) {
	    begin = t.begin;
	    end = t.end;
	    replace = t.replace;
	    return *this;
	}

	QChar begin, end;
	bool replace;
    };

    enum AutoformatType {AT_TypographicQuotes, AT_UpperCase, AT_UpperUpper};

    struct AutoformatInfo
    {
	QChar c;
	AutoformatType type;
    };

    KWAutoFormat( KWordDocument *_doc );

    void startAutoFormat( KWParag *parag, KWFormatContext *fc );
    bool doAutoFormat( KWParag *parag, KWFormatContext *fc );
    void endAutoFormat( KWParag *parag, KWFormatContext *fc );
    bool doTypographicQuotes( KWParag *parag, KWFormatContext *fc );
    bool doUpperCase( KWParag *parag, KWFormatContext *fc );
    void doSpellCheck( KWParag *parag, KWFormatContext *fc );

    void setEnabled( bool e ) { enabled = e; }
    bool isEnabled() { return enabled; }

    void configTypographicQuotes( TypographicQuotes _tq );
    void configUpperCase( bool _uc );
    void configUpperUpper( bool _uu );

    TypographicQuotes getConfigTypographicQuotes()
    { return typographicQuotes; }
    bool getConfigUpperCase()
    { return convertUpperCase; }
    bool getConfigUpperUpper()
    { return convertUpperUpper; }

    static bool isUpper( const QChar &c );
    static bool isLower( const QChar &c );
    static bool isMark( const QChar &c );
    static bool isSeparator( const QChar &c );

    void addAutoFormatEntry( KWAutoFormatEntry *entry ) {
	if ( !entry )
	    return;
	entries.insert( entry->getFind(), *entry );
	begins.append( entry->getFind()[ 0 ] );
	lengths.append( entry->getFind().length() );
	buildMaxLen();
    }

    void removeAutoFormatEntry( const QString &key ) {
	if ( entries.contains( key ) )
	    entries.remove( key );
	buildMaxLen();
    }

    QMap< QString, KWAutoFormatEntry >::Iterator firstAutoFormatEntry()
    { return entries.begin(); }

    QMap< QString, KWAutoFormatEntry >::Iterator lastAutoFormatEntry()
    { return entries.end(); }

protected:
    void buildMaxLen();

    KWordDocument *doc;
    TypographicQuotes typographicQuotes;
    bool enabled;
    KWString *tmpBuffer;
    QString spBuffer;
    KWChar *spBegin;
    bool lastWasDotSpace, convertUpperCase;
    bool lastWasUpper, convertUpperUpper;
    QMap< QString, KWAutoFormatEntry > entries;
    QValueList< QChar > begins;
    int maxlen;
    QValueList< int > lengths;

};

#endif
