/* This file is part of the KDE project
   Copyright (C) 2001 David Faure <faure@kde.org>

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

#ifndef kwtextparag_h
#define kwtextparag_h

#include "qrichtext_p.h"
#include "defs.h"
#include "kwunit.h"
#include "border.h"

#include <qstring.h>
#include <qcolor.h>
//#include <qlist.h>

using namespace Qt3;
class QDomDocument;
class KWTextFrameSet;

class KWTextParag;

class Counter
{
public:
    Counter();

    // Invalidate the internal cache. Use it whenever the number associated with this
    // counter may have changed.
    void invalidate();

    // Return the current value of the counter either as a number or text.
    int number( const KWTextParag *paragraph );
    QString text( const KWTextParag *paragraph );

    // Work out the width of the text required for this counter.
    int width( const KWTextParag *paragraph );

    // XML support.
    void load( QDomElement & element );
    void save( QDomElement & element );

    bool operator==( const Counter & c2 ) const;

    enum Numbering
    {
        NUM_NONE = 2,       // Unnumbered. Equivalent to there being
                            // no Counter structure associated with a
                            // paragraph.
        NUM_LIST = 0,       // Numbered as a list item.
        NUM_CHAPTER = 1     // Numbered as a heading.
    };
    enum Style
    {
        STYLE_NONE = 0,
        STYLE_NUM = 1, STYLE_ALPHAB_L = 2, STYLE_ALPHAB_U = 3,
        STYLE_ROM_NUM_L = 4, STYLE_ROM_NUM_U = 5, STYLE_CUSTOMBULLET = 6,
        STYLE_CUSTOM = 7, STYLE_CIRCLEBULLET = 8, STYLE_SQUAREBULLET = 9,
        STYLE_DISCBULLET = 10
    };

    Numbering m_numbering;
    Style m_style;

    // The level of the numbering.
    // Depth of 0 means the major numbering. (1, 2, 3...)
    // Depth of 1 is 1.1, 1.2, 1.3 etc.
    unsigned int m_depth;

    // Starting number.
    int m_startNumber;

    // Prefix and suffix strings.
    QString m_prefix;
    QString m_suffix;

    // The character and font for STYLE_CUSTOMBULLET.
    struct
    {
        QChar character;
        QString font;
    } m_customBullet;

    // The string STYLE_CUSTOM.
    QString m_custom;

private:

    // Return our parent paragraph, if there is such a thing. For a paragraph "1.1.",
    // the parent is the paragraph numbered "1.".
    KWTextParag *parent( const KWTextParag *paragraph );

    // The cached, calculated values for this counter:
    //
    //  VALUE                       VALUE WHEN INVALID
    //  number of this counter      -1
    //  text of this counter        QString::null
    //  width of the label          -1
    //  parent                      (KWTextParag *)-1
    struct
    {
        int number;
        QString text;
        int width;
        KWTextParag *parent;
    } m_cache;
};

/**
 * This class holds the paragraph-specific formatting information
 * It's separated from KWTextParag so that it can be copied in
 * the undo/redo history, and in KWStyle.
 */
struct KWParagLayout
{
    KWParagLayout() {}
    // Load from file
    KWParagLayout( QDomElement & parentElem );

    void save( QDomElement & parentElem );

    // From QTextParag
    int alignment;

    // From KWTextParag
    KWUnit margins[5]; // left, right, top, bottom, firstLineSpacing
    KWUnit lineSpacing;
    Border leftBorder, rightBorder, topBorder, bottomBorder;
    Counter counter;
    QString styleName;
};

/**
 * This class extends QTextParag for KWord-specific formatting stuff
 */
class KWTextParag : public QTextParag
{
public:
    KWTextParag( QTextDocument *d, QTextParag *pr = 0, QTextParag *nx = 0, bool updateIds = TRUE );
    ~KWTextParag();

    // Creates a KWParagLayout structure from our info (proper+inherited), for undo/redo purposes
    KWParagLayout createParagLayout() const;

    // Sets all the parameters from a paraglayout struct
    void setParagLayout( const KWParagLayout & layout );

    // Margins
    KWUnit margin( QStyleSheetItem::Margin m ) { return m_margins[m]; }
    const KWUnit * margins() const { return m_margins; }
    KWUnit kwLineSpacing() const { return m_lineSpacing; }

    void setMargin( QStyleSheetItem::Margin m, KWUnit _i );
    void setMargins( const KWUnit * _i );
    void setLineSpacing( KWUnit _i );

    // Borders
    Border leftBorder() const { return m_leftBorder; }
    Border rightBorder() const { return m_rightBorder; }
    Border topBorder() const { return m_topBorder; }
    Border bottomBorder() const { return m_bottomBorder; }

    void setLeftBorder( const Border & _brd ) { m_leftBorder = _brd; }
    void setRightBorder( const Border & _brd ) { m_rightBorder = _brd; }
    void setTopBorder( const Border & _brd );
    void setBottomBorder( const Border & _brd );

    // Counters are used to implement list and heading numbering/bullets.
    void setCounter( const Counter & counter );
    Counter *counter();
    void setNoCounter();

    // The space required to draw the complete counter label (i.e. the Counter for this
    // paragraph, as well as the Counters for any paragraphs above us in the numbering
    // hierarchy). See also drawLabel().
    int counterWidth() const;

    // Style
    QString styleName() const { return m_styleName; }
    void setStyleName( const QString & style ) { m_styleName = style; }

    // Public for KWStyle
    static QDomElement saveFormat( QDomDocument & doc, QTextFormat * curFormat, QTextFormat * refFormat, int pos, int len );
    static QTextFormat loadFormat( QDomElement &formatElem, QTextFormat * refFormat );

    void save( QDomElement &parentElem );
    void load( QDomElement &attributes );

    //const QList<KoTabulator> *tabList() const { return &m_tabList; }
    //void setTabList( const QList<KoTabulator> *tabList );
    //bool hasSpecialTabs() const { return m_specialTabs; }

protected:
    // This is public in QTextParag but it should be internal to KWTextParag,
    // because it's in pixels.
    virtual int topMargin() const;
    virtual int bottomMargin() const;
    virtual int leftMargin() const;
    virtual int firstLineMargin() const;
    virtual int rightMargin() const;
    virtual int lineSpacing() const;

    virtual void paint( QPainter &painter, const QColorGroup &cg, QTextCursor *cusror = 0, bool drawSelections = FALSE,
			int clipx = -1, int clipy = -1, int clipw = -1, int cliph = -1 );

    // Draw the complete label (i.e. heading/list numbers/bullets) for this paragrpah.
    virtual void drawLabel( QPainter* p, int x, int y, int w, int h, int base, const QColorGroup& cg );
    virtual void copyParagData( QTextParag *_parag );
    void invalidateCounters();
    void checkItem( QStyleSheetItem * & item, const char * name );

private:
    QStyleSheetItem * m_item;
    KWUnit m_margins[5]; // left, right, top, bottom, first line
    KWUnit m_lineSpacing;
    Border m_leftBorder, m_rightBorder, m_topBorder, m_bottomBorder;
    Counter * m_counter;
    QString m_styleName;

    //QList<KoTabulator> m_tabList;
    //bool m_specialTabs;
};

/**
 * This is our QTextDocument reimplementation, to create KWTextParag instead of QTextParags,
 * and to relate it to the text frameset it's in.
 */
class KWTextDocument : public QTextDocument
{
    Q_OBJECT
public:
    KWTextDocument( KWTextFrameSet * textfs, QTextDocument *p ) : QTextDocument( p ), m_textfs( textfs ) {
        // QTextDocument::QTextDocument creates a parag, but too early for our createParag to get called !
        // So we have to get rid of it and re-created it.
        clear( true );
    }

    virtual QTextParag * createParag( QTextDocument *d, QTextParag *pr = 0, QTextParag *nx = 0, bool updateIds = TRUE )
    {
        return new KWTextParag( d, pr, nx, updateIds );
    }

    KWTextFrameSet * textFrameSet() const { return m_textfs; }

private:
    KWTextFrameSet * m_textfs;
};

#endif
