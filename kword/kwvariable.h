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

#include <qobject.h>
#include <qstring.h>
#include <qdatetime.h>
#include <qasciidict.h>
#include <defs.h>
#include <koVariable.h>
#include <koparagcounter.h>

class KWDocument;
class KWTextFrameSet;
class KWFootNoteFrameSet;
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
    QPtrList<KAction> variableActionList();

 private:
    KWDocument *m_doc;
};

/**
 * "current page number" and "number of pages" variables
 */
class KWPgNumVariable : public QObject, public KoPgNumVariable
{
    Q_OBJECT
public:
    KWPgNumVariable( KoTextDocument *textdoc, int subtype, KoVariableFormat *varFormat ,KoVariableCollection *_varColl, KWDocument *doc );

    virtual void recalc();
    virtual void setVariableSubType( short int type);

    QPtrList<KAction> actionList();

protected slots:
    void slotChangeSubType();

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

    void setManualString( const QString & _str ) { m_varValue=QVariant(_str);}
    QString manualString() const { return m_varValue.toString();}

    virtual void resize();
    virtual void drawCustomItem( QPainter* p, int x, int y, int /*cx*/, int /*cy*/, int /*cw*/, int /*ch*/, const QColorGroup& cg, bool selected, const int offset );

    /** The frameset that contains the text for this footnote */
    KWFootNoteFrameSet * frameSet() const { return m_frameset; }
    void setFrameSet( KWFootNoteFrameSet* fs ) { Q_ASSERT( !m_frameset ); m_frameset = fs; }

    virtual void saveVariable( QDomElement &parentElem );
    virtual void load( QDomElement &elem );

    virtual QString text();
    // Nothing to do here. Numbering done by KWTextFrameSet::renumberFootNotes
    virtual void recalc() { }

    // This is a sequence number, to order footnotes. It is always set, and different for all footnotes.
    void setNum( int _num ) { m_num = _num; }
    int num() const { return m_num; }

    // The number being displayed - for auto-numbered footnotes only.
    void setNumDisplay( int val );
    int numDisplay() const { return m_numDisplay; }

    virtual void finalize();

    // The page this var is on
    int pageNum() const;
    // The current Y position of the var (in doc pt)
    double varY() const;

    virtual void setDeleted( bool del );

    void formatedNote();
    virtual QString fieldCode();
protected:
    QString applyStyle();

private:
    KWDocument *m_doc;
    NoteType m_noteType;
    KWFootNoteFrameSet* m_frameset;
    Numbering m_numberingType;
    int m_num;
    int m_numDisplay;
};

/**
 * Any variable that is a string, and whose value is automatically
 * determined - as opposed to custom variables whose value is
 * entered by the user
 */
class KWFieldVariable : public QObject, public KoFieldVariable
{
    Q_OBJECT
public:
    KWFieldVariable( KoTextDocument *textdoc, int subtype, KoVariableFormat *varFormat,KoVariableCollection *_varColl, KWDocument *_doc );

    QPtrList<KAction> actionList();

protected slots:
    void slotChangeSubType();

private:
    KWDocument *m_doc;

};

/**
 * Date-related variables
 */
class KWDateVariable : public QObject, public KoDateVariable
{
    Q_OBJECT
public:
    KWDateVariable( KoTextDocument *textdoc, int subtype, KoVariableFormat *varFormat, KoVariableCollection *_varColl, KWDocument *_doc );

    QPtrList<KAction> actionList();

protected slots:
    void slotChangeSubType();
    void slotChangeFormat();

private:
    KWDocument *m_doc;

};

/**
 * Time-related variables
 */
class KWTimeVariable : public QObject, public KoTimeVariable
{

    Q_OBJECT
public:
    KWTimeVariable( KoTextDocument *textdoc, int subtype, KoVariableFormat *varFormat, KoVariableCollection *_varColl, KWDocument *_doc );

    QPtrList<KAction> actionList();

protected slots:
    void slotChangeSubType();
    void slotChangeFormat();

private:
    KWDocument *m_doc;

};

#endif
