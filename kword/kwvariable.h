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
#include <qasciidict.h>
#include <defs.h>
#include <koVariable.h>
#include <koparagcounter.h>

class KWDocument;
class KWTextFrameSet;
class KoVariable;
class KoPgNumVariable;
class KoMailMergeVariable;
class QDomElement;
class KoTextFormat;


class KWVariableSettings : public KoVariableSettings
{
 public:
    KWVariableSettings();
    virtual ~KWVariableSettings() {}
    virtual void save( QDomElement &parentElem );
    virtual void load( QDomElement &elem );
    void changeFootNoteCounter( KoParagCounter _c );
    void changeEndNoteCounter( KoParagCounter _c );
    KoParagCounter endNoteCounter() const { return m_endNoteCounter;}
    KoParagCounter footNoteCounter() const { return m_footNoteCounter;}
 private:
    KoParagCounter m_footNoteCounter;
    KoParagCounter m_endNoteCounter;
};

class KWVariableCollection : public KoVariableCollection
{
 public:
    KWVariableCollection(KWVariableSettings *_settings);
    virtual KoVariable *createVariable( int type, int subtype, KoVariableFormatCollection * coll, KoVariableFormat *varFormat,KoTextDocument *textdoc, KoDocument * doc );
};

/**
 * "current page number" and "number of pages" variables
 */
class KWPgNumVariable : public KoPgNumVariable
{
public:
    KWPgNumVariable( KoTextDocument *textdoc, int subtype, KoVariableFormat *varFormat ,KoVariableCollection *_varColl, KWDocument *doc );

    virtual void recalc();
    virtual void setVariableSubType( short int type);
 private:
    KWDocument *m_doc;
};


/**
 * Mail Merge variable
 */
class KWMailMergeVariable : public KoMailMergeVariable
{
public:
    KWMailMergeVariable( KoTextDocument *textdoc, const QString &name, KoVariableFormat *varFormat,KoVariableCollection *_varColl, KWDocument *doc );

    virtual QString text();
    virtual QString value() const;
    virtual void recalc();
 private:
    KWDocument *m_doc;
};

/**
 * The variable showing the footnote number in superscript, in the text.
 */
class KWFootNoteVariable : public KoVariable
{
public:
    KWFootNoteVariable( KoTextDocument *textdoc, KoVariableFormat *varFormat, KoVariableCollection *varColl, KWDocument *doc );
    virtual VariableType type() const
    { return VT_FOOTNOTE; }
    enum Numbering {Auto, Manual};

    void setNoteType( NoteType _noteType ) { m_noteType = _noteType;}
    NoteType noteType() const {return m_noteType; }

    void setNumberingType( Numbering _type );
    Numbering numberingType() const { return m_numberingType;}

    void setManualString( const QString & _str ) { setValue(_str);}
    QString manualString() const { return m_varValue.toString();}

    virtual void drawCustomItem( QPainter* p, int x, int y, int /*cx*/, int /*cy*/, int /*cw*/, int /*ch*/, const QColorGroup& cg, bool selected, const int offset );

    /** The frameset that contains the text for this footnote */
    KWTextFrameSet * frameSet() const { return m_frameset; }
    void setFrameSet( KWTextFrameSet* fs ) { Q_ASSERT( !m_frameset ); m_frameset = fs; }

    virtual void saveVariable( QDomElement &parentElem );
    virtual void load( QDomElement &elem );

    virtual QString text();
    // Nothing to do here. Numbering done by KWTextFrameSet::renumberFootNotes
    virtual void recalc() { }

    // Important: we need a sequence number, to order footnotes.
    // Don't remove this when implementing the QVariant-based solution ;)
    void setNum( int _num ) { m_num = _num; }
    int num() const { return m_varValue.toInt(); }
    void setValue( const QVariant & value ) { m_varValue = value;}

    // Called whenever this item is being moved by the text formatter
    virtual void move( int x, int y );

    virtual void setDeleted( bool del );

private:
    KWDocument *m_doc;
    NoteType m_noteType;
    KWTextFrameSet* m_frameset;
    Numbering m_numberingType;
    int m_num;
};

#endif
