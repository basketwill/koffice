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

#ifndef variable_h
#define variable_h

#include <qstring.h>
#include <qdatetime.h>

class KWTextFrameSet;
class KWDocument;
class KWVariable;
class QDomElement;

enum VariableType { VT_DATE_FIX = 0, VT_DATE_VAR = 1, VT_TIME_FIX = 2, VT_TIME_VAR = 3, VT_PGNUM = 4,
                    VT_NUMPAGES = 5, VT_FILENAME = 6, VT_AUTHORNAME = 7, VT_EMAIL =8
 ,VT_COMPANYNAME=9, VT_CUSTOM = 10, VT_SERIALLETTER = 11 , VT_NONE };
//enum VariableFormatType { VFT_DATE = 0, VFT_TIME = 1, VFT_PGNUM = 2, VFT_NUMPAGES = 3, VFT_CUSTOM = 4,
//                          VFT_SERIALLETTER = 5 };

/**
 * Class: KWVariableFormat
 * Base class for a variable format - held by KWDocument.
 * The variable itself is implemented by KWVariable
 * The reason for formats to be separated is that it allows to
 * customize the formats - for instance, KWVariablePgNumFormat has
 * 'pre' and 'post' variables.
 */
class KWVariableFormat
{
public:
    KWVariableFormat() {}
    virtual ~KWVariableFormat() {}
//    virtual VariableFormatType getType() const = 0;
    virtual QString convert( KWVariable *_var ) = 0;
};

/******************************************************************/
/* Class: KWVariableDateFormat                                    */
/******************************************************************/
class KWVariableDateFormat : public KWVariableFormat
{
public:
    KWVariableDateFormat() : KWVariableFormat() {}

//    virtual VariableFormatType getType() const
//    { return VFT_DATE; }

    virtual QString convert( KWVariable *_var );

};

/******************************************************************/
/* Class: KWVariableFileNameFormat                                */
/******************************************************************/
class KWVariableFileNameFormat : public KWVariableFormat
{
public:
    KWVariableFileNameFormat() : KWVariableFormat() {}

//    virtual VariableFormatType getType() const
//    { return VFT_TIME; }

    virtual QString convert( KWVariable *_var );

};

/******************************************************************/
/* Class: KWVariableAuthorNameFormat                              */
/******************************************************************/
class KWVariableAuthorNameFormat : public KWVariableFormat
{
public:
    KWVariableAuthorNameFormat() : KWVariableFormat() {}

//    virtual VariableFormatType getType() const
//    { return VFT_TIME; }

    virtual QString convert( KWVariable *_var );

};

/******************************************************************/
/* Class: KWVariableEmailFormat                                   */
/******************************************************************/
class KWVariableEmailFormat : public KWVariableFormat
{
public:
    KWVariableEmailFormat() : KWVariableFormat() {}

//    virtual VariableFormatType getType() const
//    { return VFT_TIME; }

    virtual QString convert( KWVariable *_var );

};

/******************************************************************/
/* Class: KWVariableCompanyNameFormat                             */
/******************************************************************/
class KWVariableCompanyNameFormat : public KWVariableFormat
{
public:
    KWVariableCompanyNameFormat() : KWVariableFormat() {}

//    virtual VariableFormatType getType() const
//    { return VFT_TIME; }

    virtual QString convert( KWVariable *_var );

};

/******************************************************************/
/* Class: KWVariableTimeFormat                                    */
/******************************************************************/
class KWVariableTimeFormat : public KWVariableFormat
{
public:
    KWVariableTimeFormat() : KWVariableFormat() {}

//    virtual VariableFormatType getType() const
//    { return VFT_TIME; }

    virtual QString convert( KWVariable *_var );

};

/******************************************************************/
/* Class: KWVariablePgNumFormat                                   */
/******************************************************************/
class KWVariablePgNumFormat : public KWVariableFormat
{
public:
    KWVariablePgNumFormat() { pre = "-"; post = "-"; }

//    virtual VariableFormatType getType() const
//    { return VFT_PGNUM; }

    virtual QString convert( KWVariable *_var );

    // Needs a UI !
    void setPre( const QString &_pre ) { pre = _pre; }
    void setPost( const QString &_post ) { pre = _post; }

    QString getPre() const { return pre; }
    QString getPost() const { return post; }

protected:
    QString pre, post;

};

/******************************************************************/
/* Class: KWVariableCustomFormat                                   */
/******************************************************************/
class KWVariableCustomFormat : public KWVariableFormat
{
public:
    KWVariableCustomFormat() {}

//    virtual VariableFormatType getType() const
//    { return VFT_CUSTOM; }

    virtual QString convert( KWVariable *_var );

};

/******************************************************************/
/* Class: KWVariableSerialLetterFormat                            */
/******************************************************************/
class KWVariableSerialLetterFormat : public KWVariableFormat
{
public:
    KWVariableSerialLetterFormat() {}

//    virtual VariableFormatType getType() const
//    { return VFT_SERIALLETTER; }

    virtual QString convert( KWVariable *_var );

};


// ----------------------------------------------------------------------------------------------

#include <qrichtext_p.h>
using namespace Qt3;


/******************************************************************/
/* Class: KWVariable                                              */
/******************************************************************/

class KWVariable : public QTextCustomItem
{
public:
    KWVariable( KWTextFrameSet *fs, KWVariableFormat *_varFormat );
    virtual ~KWVariable();

    /* Clones a variable */
/*    virtual KWVariable *copy() {
        KWVariable *v = new KWVariable( doc );
        v->setVariableFormat( varFormat );
        v->setInfo( frameSetNum, frameNum, pageNum, parag );
        return v;
        }*/

    virtual VariableType getType() const
    { return VT_NONE; }

    // QTextCustomItem stuff
    Placement placement() const { return PlaceInline; }
    void adjustToPainter( QPainter* );
    int widthHint() const { return width; }
    int minimumWidth() const { return width; }
    void draw( QPainter* p, int x, int y, int cx, int cy, int cw, int ch, const QColorGroup& cg );


// Much too dangerous.
//    void setVariableFormat( KWVariableFormat *_varFormat, bool _deleteOld = false )
//    { if ( _deleteOld && varFormat ) delete varFormat; varFormat = _varFormat; }

//    KWVariableFormat *getVariableFormat() const
//    { return varFormat; }

    QString getText()
    { return varFormat->convert( this ); }

    // parag * is a bit dangerous. paragId() maybe?
    virtual void setInfo( int _frameSetNum, int _frameNum, int _pageNum /*, KWParag * _parag*/ )
    { frameSetNum = _frameSetNum; frameNum = _frameNum; pageNum = _pageNum; /*parag = _parag;*/ }

    virtual void recalc() {}

    virtual void save( QDomElement &parentElem );
    virtual void load( QDomElement &elem );

protected:
    KWDocument *doc;
    KWVariableFormat *varFormat;
    QString text;
    int frameSetNum, frameNum, pageNum;
    //KWParag *parag;
};

/******************************************************************/
/* Class: KWPgNumVariable                                         */
/******************************************************************/

class KWPgNumVariable : public KWVariable
{
public:
    KWPgNumVariable( KWTextFrameSet *fs, KWVariableFormat *_varFormat )
        : KWVariable( fs, _varFormat ) { pgNum = 0; }

/*    virtual KWVariable *copy() {
        KWPgNumVariable *var = new KWPgNumVariable( doc );
        var->setVariableFormat( varFormat );
        var->setInfo( frameSetNum, frameNum, pageNum, parag );
           return var;
           }*/

    virtual VariableType getType() const
    { return VT_PGNUM; }

    virtual void recalc() { pgNum = pageNum; }
    int getPgNum() const { return pgNum; }

    virtual void save( QDomElement &parentElem );
    virtual void load( QDomElement &elem );

protected:
    int pgNum;

};

/******************************************************************/
/* Class: KWDateVariable                                          */
/******************************************************************/

class KWDateVariable : public KWVariable
{
public:
    KWDateVariable( KWTextFrameSet *fs, bool _fix, QDate _date, KWVariableFormat *_varFormat );
    //KWDateVariable( KWTextFrameSet *fs ) : KWVariable( fs ) {}

/*    virtual KWVariable *copy() {
        KWDateVariable *var = new KWDateVariable( doc, fix, date );
        var->setVariableFormat( varFormat );
        var->setInfo( frameSetNum, frameNum, pageNum, parag );
        return var;
        }*/

    virtual VariableType getType() const
    { return fix ? VT_DATE_FIX : VT_DATE_VAR; }

    QDate getDate() const { return date; }
    void setDate( QDate _date ) { date = _date; }

    virtual void save( QDomElement &parentElem );
    virtual void load( QDomElement &elem );

protected:
    QDate date;
    bool fix;

};

/******************************************************************/
/* Class: KWTimeVariable                                          */
/******************************************************************/

class KWTimeVariable : public KWVariable
{
public:
    KWTimeVariable( KWTextFrameSet *fs, bool _fix, QTime _time, KWVariableFormat *_varFormat );
    //KWTimeVariable( KWTextFrameSet *fs ) : KWVariable( fs ) {}

/*    virtual KWVariable *copy() {
        KWTimeVariable *var = new KWTimeVariable( doc, fix, time );
        var->setVariableFormat( varFormat );
        var->setInfo( frameSetNum, frameNum, pageNum, parag );
        return var;
        }*/

    virtual VariableType getType() const
    { return fix ? VT_TIME_FIX : VT_TIME_VAR; }

    QTime getTime() const { return time; }
    void setTime( QTime _time ) { time = _time; }

    virtual void save( QDomElement &parentElem );
    virtual void load( QDomElement &elem );

protected:
    QTime time;
    bool fix;

};

/******************************************************************/
/* Class: KWFileNameVariable                                      */
/******************************************************************/

class KWFileNameVariable : public KWVariable
{
public:
    KWFileNameVariable( KWTextFrameSet *fs, const QString &_fileName, KWVariableFormat *_varFormat );

/*    virtual KWVariable *copy() {
        KWFileNameVariable *var = new KWFileNameVariable( doc, filename );
        var->setVariableFormat( varFormat );
        var->setInfo( frameSetNum, frameNum, pageNum, parag );
        return var;
        }*/

    virtual VariableType getType() const
    { return VT_FILENAME; }

    QString getFileName() const { return filename; }
    void setFileName( QString _filename ) { filename = _filename; }

    virtual void save( QDomElement &parentElem );
    virtual void load( QDomElement &elem );

protected:
    QString filename;
};

/******************************************************************/
/* Class: KWNameAuthorVariable                                    */
/******************************************************************/

class KWNameAuthorVariable : public KWVariable
{
public:
    KWNameAuthorVariable( KWTextFrameSet *fs, const QString &_authorName, KWVariableFormat *_varFormat );

/*    virtual KWVariable *copy() {
        KWFileNameVariable *var = new KWAuthorNameVariable( doc, filename );
        var->setVariableFormat( varFormat );
        var->setInfo( frameSetNum, frameNum, pageNum, parag );
        return var;
        }*/

    virtual VariableType getType() const
    { return VT_AUTHORNAME; }

    QString getAuthorName() const { return authorname; }
    void setAuthorName( QString _authorname ) { authorname = _authorname; }

    virtual void save( QDomElement &parentElem );
    virtual void load( QDomElement &elem );

protected:
    QString authorname;
};

/******************************************************************/
/* Class: KWEmailVariable                                         */
/******************************************************************/

class KWEmailVariable : public KWVariable
{
public:
    KWEmailVariable( KWTextFrameSet *fs, const QString &_email, KWVariableFormat *_varFormat );

/*    virtual KWVariable *copy() {
        KWFileNameVariable *var = new KWEmailVariable( doc, filename );
        var->setVariableFormat( varFormat );
        var->setInfo( frameSetNum, frameNum, pageNum, parag );
        return var;
        }*/

    virtual VariableType getType() const
    { return  VT_EMAIL; }

    QString getEmail() const { return email; }
    void setEmail( QString _email ) { email = _email; }

    virtual void save( QDomElement &parentElem );
    virtual void load( QDomElement &elem );

protected:
    QString email;
};

/******************************************************************/
/* Class: KWCompanyNameVariable                                   */
/******************************************************************/

class KWCompanyNameVariable : public KWVariable
{
public:
    KWCompanyNameVariable( KWTextFrameSet *fs, const QString &_companyname, KWVariableFormat *_varFormat );

/*    virtual KWVariable *copy() {
        KWFileNameVariable *var = new KWEmailVariable( doc, filename );
        var->setVariableFormat( varFormat );
        var->setInfo( frameSetNum, frameNum, pageNum, parag );
        return var;
        }*/

    virtual VariableType getType() const
    { return  VT_COMPANYNAME; }

    QString getCompanyName() const { return companyname; }
    void setCompanyName( QString _companyname ) { companyname = _companyname; }

    virtual void save( QDomElement &parentElem );
    virtual void load( QDomElement &elem );

protected:
    QString companyname;
};

/******************************************************************/
/* Class: KWCustomVariable                                        */
/******************************************************************/

class KWCustomVariable : public KWVariable
{
public:
    KWCustomVariable( KWTextFrameSet *fs, const QString &name_, KWVariableFormat *_varFormat );
    //KWCustomVariable( KWTextFrameSet *fs ) : KWVariable( fs ) {}

/*    virtual KWVariable *copy() {
        KWCustomVariable *var = new KWCustomVariable( doc, name );
        var->setVariableFormat( varFormat );
        var->setInfo( frameSetNum, frameNum, pageNum, parag );
        return var;
        }*/

    virtual VariableType getType() const
    { return VT_CUSTOM; }

    virtual void save( QDomElement &parentElem );
    virtual void load( QDomElement &elem );

    virtual QString getName() const { return name; }
    virtual QString getValue() const;

    virtual void setValue( const QString &v );

protected:
    QString name;

};

/******************************************************************/
/* Class: KWSerialLetterVariable                                  */
/******************************************************************/

class KWSerialLetterVariable : public KWVariable
{
public:
    KWSerialLetterVariable( KWTextFrameSet *fs, const QString &name_, KWVariableFormat *_varFormat );
    //KWSerialLetterVariable( KWTextFrameSet *fs ) : KWVariable( fs ) {}

/*    virtual KWVariable *copy() {
        KWSerialLetterVariable *var = new KWSerialLetterVariable( doc, name );
        var->setVariableFormat( varFormat );
        var->setInfo( frameSetNum, frameNum, pageNum, parag );
        return var;
        }*/

    virtual VariableType getType() const
    { return VT_SERIALLETTER; }

    virtual void save( QDomElement &parentElem );
    virtual void load( QDomElement &elem );

    virtual QString getName() const { return name; }
    virtual QString getValue() const;

protected:
    QString name;

};

#endif
