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

#include "kwvariable.h"

#include "kwtextframeset.h"
#include "kwtextdocument.h"
#include <kovariable.h>
#include "mailmerge.h"
#include "kwdoc.h"
#include "kwcommand.h"
#include "kwviewmode.h"
#include <klocale.h>
#include <kdebug.h>
#include <qdom.h>
#include <qtimer.h>

KWVariableSettings::KWVariableSettings() : KoVariableSettings()
{
    m_footNoteCounter.setSuffix( QString::null );
    m_endNoteCounter.setSuffix( QString::null );
    // By default endnotes are numbered in lowercase roman numbers, in other WPs.
    m_endNoteCounter.setStyle( KoParagCounter::STYLE_ROM_NUM_L );
}

void KWVariableSettings::changeFootNoteCounter( KoParagCounter _c )
{
    m_footNoteCounter = _c;
}

void KWVariableSettings::changeEndNoteCounter( KoParagCounter _c )
{
    m_endNoteCounter = _c;
}


void KWVariableSettings::save( QDomElement &parentElem )
{
    KoVariableSettings::save( parentElem );
    QDomDocument doc = parentElem.ownerDocument();
    QDomElement footNoteSettingElem = doc.createElement( "FOOTNOTESETTING" );
    parentElem.appendChild( footNoteSettingElem );
    m_footNoteCounter.save( footNoteSettingElem );
    QDomElement endNoteSettingElem = doc.createElement( "ENDNOTESETTING" );
    parentElem.appendChild( endNoteSettingElem );
    m_endNoteCounter.save( endNoteSettingElem );
}

void KWVariableSettings::load( QDomElement &elem )
{
    KoVariableSettings::load( elem );
    QDomElement footNoteSettings = elem.namedItem( "FOOTNOTESETTING" ).toElement();
    if ( !footNoteSettings.isNull() )
        m_footNoteCounter.load( footNoteSettings );
    QDomElement endNoteSettings = elem.namedItem( "ENDNOTESETTING" ).toElement();
    if ( !endNoteSettings.isNull() )
        m_endNoteCounter.load( endNoteSettings );
}

KWVariableCollection::KWVariableCollection(KWVariableSettings *_setting, KoVariableFormatCollection* coll)
    : KoVariableCollection(_setting, coll)
{
}


KoVariable *KWVariableCollection::createVariable( int type, short int subtype, KoVariableFormatCollection * coll, KoVariableFormat *varFormat,KoTextDocument *textdoc, KoDocument * doc, int _correct,bool _forceDefaultFormat, bool loadFootNote )
{
    KWDocument *m_doc = static_cast<KWDocument *>(doc);
    KoVariable * var = 0L;
    switch(type) {
    case VT_PGNUM:
        if ( !varFormat )
            varFormat = (subtype == KoPgNumVariable::VST_CURRENT_SECTION) ? coll->format("STRING") : coll->format("NUMBER");
        var = new KWPgNumVariable( textdoc, subtype, varFormat, this, m_doc );
        break;
    case VT_MAILMERGE:
	var = new KWMailMergeVariable( textdoc, QString::null, coll->format("STRING"), this, m_doc );
        break;
    case VT_FOOTNOTE:
        if ( !loadFootNote )
            return 0L;
        if ( !varFormat )
            varFormat =  coll->format("STRING");
        var = new KWFootNoteVariable( textdoc, varFormat, this, m_doc );
        break;
    default:
        return KoVariableCollection::createVariable( type, subtype, coll, varFormat, textdoc, doc, _correct, _forceDefaultFormat );
    }
    return var;
}

/******************************************************************/
/* Class: KWPgNumVariable                                         */
/******************************************************************/
KWPgNumVariable::KWPgNumVariable( KoTextDocument *textdoc, int subtype, KoVariableFormat *varFormat ,KoVariableCollection *_varColl, KWDocument *doc  )
    : KoPgNumVariable( textdoc, subtype, varFormat ,_varColl ), m_doc(doc)
{
}

void KWPgNumVariable::recalc()
{
    if ( m_doc->viewMode()->type() == "ModeText")
    {
        //necessary to resize it in this mode because in this mode
        //we don't call KWTextFrameSet::drawFrame()
        resize();
        return;
    }

    if ( m_subtype == VST_PGNUM_TOTAL )
    {
        m_varValue = QVariant(m_doc->numPages()+m_varColl->variableSetting()->startingPage()-1);
        resize();
    }
    // The other cases are handled by the more dynamic code in KWTextFrameSet::drawFrame()
    // But we don't want to keep a width of -1 ...
    if ( width == -1 )
        width = 0;
}

QString KWPgNumVariable::text(bool realValue)
{
    if (m_varColl->variableSetting()->displayFieldCode()&& !realValue)
        return fieldCode();
    // #### ??? What?
    else if ( m_subtype !=  VST_CURRENT_SECTION && m_doc->viewMode()->type() == "ModeText" && !realValue)
        return fieldCode();
    else
        return m_varFormat->convert( m_varValue );
}

/******************************************************************/
/* Class: KWMailMergeVariable                                  */
/******************************************************************/
KWMailMergeVariable::KWMailMergeVariable( KoTextDocument *textdoc, const QString &name, KoVariableFormat *varFormat,KoVariableCollection *_varColl, KWDocument *doc  )
    : KoMailMergeVariable( textdoc, name, varFormat,_varColl ), m_doc(doc)
{
}

QString KWMailMergeVariable::value() const
{
    return m_doc->getMailMergeDataBase()->getValue( m_varValue.toString() );
}

QString KWMailMergeVariable::text(bool realValue)
{
    if (m_varColl->variableSetting()->displayFieldCode()&& !realValue)
        return fieldCode();
    // ## should use a format maybe
    QString v = value();
    if (m_doc->getMailMergeDataBase()->isSampleRecord() )
        return "<" + v + ">";
    return v;
}

void KWMailMergeVariable::recalc()
{
    resize();
}

/////////////

KWFootNoteVariable::KWFootNoteVariable( KoTextDocument *textdoc, KoVariableFormat *varFormat, KoVariableCollection *varColl, KWDocument *doc )
    : KoVariable( textdoc, varFormat, varColl ),
      m_doc(doc),
      m_frameset( 0L ),
      m_numberingType( Auto ),
      m_num( -1 ),
      m_numDisplay( -1 )
{
    m_varValue = QVariant( QString::null );
}

void KWFootNoteVariable::setNumberingType( Numbering _type )
{
    m_numberingType = _type;
    //delete m_varFormat;
    setVariableFormat(m_doc->variableFormatCollection()->format("STRING"));
}

void KWFootNoteVariable::loadOasis( const QDomElement &elem, KoOasisContext& context )
{
    //TODO
}

void KWFootNoteVariable::saveOasis( KoXmlWriter& writer, KoSavingContext& context ) const
{
    //TODO
}

void KWFootNoteVariable::saveVariable( QDomElement &parentElem )
{
    QDomElement footnoteElem = parentElem.ownerDocument().createElement( "FOOTNOTE" );
    parentElem.appendChild( footnoteElem );
    //footnoteElem.setAttribute( "subtype", 0 );
    if ( m_numberingType == Auto )
        footnoteElem.setAttribute( "value", m_numDisplay );
    else
        footnoteElem.setAttribute( "value", m_varValue.toString() );
    footnoteElem.setAttribute( "notetype", m_noteType == FootNote ? "footnote" : "endnote" );
    footnoteElem.setAttribute( "numberingtype", m_numberingType == Auto ? "auto" : "manual" );
    Q_ASSERT( m_frameset );
    footnoteElem.setAttribute( "frameset", m_frameset->getName() );
}

void KWFootNoteVariable::load( QDomElement &elem )
{
    KoVariable::load( elem );
    QDomElement footnoteElem = elem.namedItem( "FOOTNOTE" ).toElement();
    if (!footnoteElem.isNull())
    {
        //m_subtype = footnoteElem.attribute("subtype").toInt();
        QString str = footnoteElem.attribute("notetype").lower();
        if ( str == "footnote" )
            m_noteType = FootNote;
        else if ( str == "endnote" )
            m_noteType = EndNote;
        else
            kdWarning() << "Unknown footnote type: '" << str << "'" << endl;

        str = footnoteElem.attribute("numberingtype").lower();
        if ( str == "auto" )
            m_numberingType = Auto;
        else if ( str == "manual")
            m_numberingType = Manual;
        else
            kdWarning() << "Unknown footnote numbering: '" << str << "'" << endl;

        if ( m_numberingType == Auto )
        {
            m_numDisplay = footnoteElem.attribute("value").toInt();
            formatedNote();
        }
        else
            m_varValue = QVariant(footnoteElem.attribute("value"));

        str = footnoteElem.attribute("frameset");
        m_doc->addFootNoteRequest( str, this );
    }
}

void KWFootNoteVariable::formatedNote()
{
    if ( m_numberingType == Auto )
    {
        m_varValue = QVariant(applyStyle());
    }
}

QString KWFootNoteVariable::applyStyle()
{
    KWVariableSettings* settings = static_cast<KWVariableSettings*>(m_varColl->variableSetting());
    KoParagCounter tmpCounter = (m_noteType == FootNote) ? settings->footNoteCounter() : settings->endNoteCounter();

    QString tmp;
    int val = m_numDisplay + tmpCounter.startNumber()-1;
    Q_ASSERT( val >= 0 );
    if ( val < 0 ) // let's not go into makeRomanNumber with a negative number :}
        return i18n("ERROR");
    switch ( tmpCounter.style() )
    {
    case KoParagCounter::STYLE_NUM:
        tmp.setNum( val );
        break;
    case KoParagCounter::STYLE_ALPHAB_L:
        tmp=KoParagCounter::makeAlphaLowerNumber( val );
        break;
    case KoParagCounter::STYLE_ALPHAB_U:
        tmp=KoParagCounter::makeAlphaUpperNumber( val );
        break;
    case KoParagCounter::STYLE_ROM_NUM_L:
        tmp = KoParagCounter::makeRomanNumber( val ).lower();
        break;
    case KoParagCounter::STYLE_ROM_NUM_U:
        tmp = KoParagCounter::makeRomanNumber( val ).upper();
        break;
    case KoParagCounter::STYLE_CUSTOMBULLET:
        //todo change font
        tmp = tmpCounter.customBulletCharacter();
        break;
    default:
        tmp.setNum( val );
        break;
    }

    tmp.prepend( tmpCounter.prefix() );
    tmp.append( tmpCounter.suffix() );
    return tmp;
}

QString KWFootNoteVariable::text(bool realValue)
{
    if (m_varColl->variableSetting()->displayFieldCode()&& !realValue)
        return fieldCode();
    return m_varFormat->convert( m_varValue );
}

void KWFootNoteVariable::setNumDisplay( int val )
{
    m_numDisplay = val;
    if ( val != -1 ) // -1 is used to 'invalidate so that renumberFootNotes recalcs'
        formatedNote();
}

QString KWFootNoteVariable::fieldCode()
{
    return (noteType()==FootNote) ?i18n("Footnote"):i18n("Endnote");
}

void KWFootNoteVariable::drawCustomItem( QPainter* p, int x, int y, int wpix, int hpix, int ascentpix, int /*cx*/, int /*cy*/, int /*cw*/, int /*ch*/, const QColorGroup& cg, bool selected, int _offset, bool drawingShadow )
{
    KoTextFormat * fmt = format();
    KoZoomHandler * zh = textDocument()->paintingZoomHandler();

    // Force drawing as "superscript" - hmm, the formatting will use too big font metrics though.
    QFont font( fmt->screenFont( zh ) );
    int pointSize = ( ( font.pointSize() * 2 ) / 3 );
    font.setPointSize( pointSize );

    int offset = _offset;
    if ( offset == 0 )
    {
        int h = zh->layoutUnitToPixelY( /*_y HACK,*/ height );
        offset = -( h - QFontMetrics(font).height() );
    }

    QColor textColor( fmt->color() );
    drawCustomItemHelper( p, x, y, wpix, hpix, ascentpix, cg, selected, offset, fmt, font, textColor, drawingShadow );
}

void KWFootNoteVariable::finalize()
{
    Q_ASSERT( m_frameset );
    if (!m_frameset )
        return;
    Q_ASSERT( !m_frameset->isDeleted() );
    if ( m_frameset->isDeleted() )
        return;

    //kdDebug(32001) << "KWFootNoteVariable::finalize" << endl;

    int pageNum = this->pageNum();
    if ( pageNum == -1 )
        return;

    KWFrame* footNoteFrame = m_frameset->frame( 0 );
    int framePage = footNoteFrame->pageNum();
    if ( framePage != pageNum )
    {
        //kdDebug(32001) << "Footnote var '" << text() << "' at page " << pageNum << ", footnote frame at page " << framePage << " -> abortFormatting() and recalcFrames()" << endl;
        KWTextFrameSet * fs = static_cast<KWTextDocument *>(textDocument())->textFrameSet();
        fs->textObject()->abortFormatting();

        // abortFormatting is a bool in kotextobject. So we need to return there before
        // starting text layout again.
        m_doc->delayedRecalcFrames( QMIN( pageNum, framePage ) );
	m_doc->delayedRepaintAllViews();
    }
}

void KWFootNoteVariable::resize()
{
    if ( m_deleted )
        return;
    KoTextFormat *fmt = format();
    QFont font( fmt->refFont() ); // LU font
    if ( fmt->vAlign() == KoTextFormat::AlignNormal ) // if it's still normal...
    {
        int pointSize = ( ( font.pointSize() * 2 ) / 3 ); // ...force superscript
        font.setPointSize( pointSize );
    }
    QFontMetrics fm( font );
    QString txt = text();
    width = 0;
    for ( int i = 0 ; i < (int)txt.length() ; ++i )
        width += fm.charWidth( txt, i ); // size at 100%
    // zoom to LU
    width = qRound( KoTextZoomHandler::ptToLayoutUnitPt( width ) );
    height = fmt->height();
    m_ascent = fmt->ascent();
    //kdDebug() << "KWFootNoteVariable::resize text=" << txt << " width=" << width << " height=" << height << endl;
}

void KWFootNoteVariable::setDeleted( bool del )
{
    kdDebug() << "KWFootNoteVariable::setDeleted " << del << endl;
    if ( del )
    {
        Q_ASSERT( m_frameset );
        if ( m_frameset ) {
            m_frameset->deleteAllFrames(); // Important, because we don't want to save it!
            m_frameset->setVisible( false );
        }
    }
    else
    {
        Q_ASSERT( m_frameset );
        if ( m_frameset ) {
            kdDebug() << "Making frameset " << m_frameset << " visible" << endl;
            m_frameset->setVisible( true );
            if ( m_frameset->isDeleted() )
                m_frameset->createInitialFrame( 0 ); // Page number shouldn't matter (see recalcFrames below).
            Q_ASSERT( m_frameset->isVisible() );
        }
    }

    // hmm, maybe compress all the stuff below and do only once
    // (e.g. when deleting multiple footnotes)?
    // (but we can't really delay it with a sst, the formatMore after undo/redo
    // needs this to be done already, I think). Bah.

    // Re-number footnote variables
    KWTextFrameSet * textfs = static_cast<KWTextDocument *>(textDocument())->textFrameSet();
    textfs->renumberFootNotes();

    m_doc->recalcFrames();

    if (!del)
        m_frameset->layout(); // format its text, so that it resizes the frame
    KoVariable::setDeleted( del );
    // Does this compress? Probably not.
    m_doc->delayedRepaintAllViews();
}

int KWFootNoteVariable::pageNum() const
{
    int page = static_cast<int>(varY() / m_doc->ptPaperHeight());
    Q_ASSERT( page <= m_doc->numPages()-1 );
    return page;
}

double KWFootNoteVariable::varY() const
{
    // Find out the position of the footnote variable in document coordinates.
    int paragy = paragraph()->rect().y();
    KWTextFrameSet * fs = static_cast<KWTextDocument *>(textDocument())->textFrameSet();
    if ( !fs->hasFramesInPageArray() ) // we need it for internalToDocument
    {
        kdDebug(32001) << "KWFootNoteVariable::varY too early, no updateFrames yet" << endl;
        return 0; // this happens on loading - frame layout is done before text layout
    }
    // What we need is "has never been formatted". Not "has just been invalidated"...
    //if ( !paragraph()->isValid() )
    //{
    //    kdDebug(32001) << "KWFootNoteVariable::varY called but paragraph " << paragraph()->paragId() << " not valid" << endl;
    //    return 0;
    //}
    KoPoint dPoint;
    //kdDebug(32001) << "KWFootNoteVariable::pageNum position of variable (LU): " << QPoint( x(), paragy + y() + height ) << endl;
    KWFrame* containingFrame = fs->internalToDocument( QPoint( x(), paragy + y() + height ), dPoint );
    if ( containingFrame )
    {
        // Ok, the (bottom of the) footnote variable is at dPoint.
        double varY = dPoint.y();
        //kdDebug(32001) << " found containingFrame " << containingFrame << " page:" << containingFrame->pageNum() << "  varY=" << varY << endl;
        //int pageNum = containingFrame->pageNum(); // and at page pageNum
        return varY;
    } else
    {
        // This can happen if the page hasn't been created yet
        //kdDebug(32001) << "KWFootNoteVariable::pageNum internalToDocument returned 0L for " << x << ", " << y+paragy << endl;
        return 0;
    }
}
