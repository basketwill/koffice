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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KWAnchor.h"
#include "KWTextFrameSet.h"
#include "KWDocument.h"
#include "KWViewMode.h"
#include "KWView.h"
#include <KoXmlWriter.h>
#include <kdebug.h>
//Added by qt3to4:
#include <QList>

//#define DEBUG_DRAWING

KWAnchor::KWAnchor( KoTextDocument *textDocument, KWFrameSet * frameset, int frameNum )
    : KoTextCustomItem( textDocument),
      m_frameset( frameset ),
      m_frameNum( frameNum )
{
}

KWAnchor::~KWAnchor()
{
    kDebug(32001) <<"KWAnchor::~KWAnchor";
}

void KWAnchor::setFormat( KoTextFormat* format )
{
    m_frameset->setAnchorFormat( format, m_frameNum );
}

void KWAnchor::finalize()
{
#if 0
    if ( m_deleted )
        return;

    int paragx = paragraph()->rect().x();
    int paragy = paragraph()->rect().y();
    kDebug(32001) << this <<" KWAnchor::finalize" << x() <<"," << y() <<" paragx=" << paragx <<" paragy=" << paragy;

    KWTextFrameSet * fs = static_cast<KWTextDocument *>(textDocument())->textFrameSet();
    KoPoint dPoint;
    if ( fs->internalToDocument( QPoint( x()+paragx, y()+paragy ), dPoint ) )
    {
        //kDebug(32001) <<"KWAnchor::finalize moving frame to" << dPoint.x() <<"," << dPoint.y();
        // Move the frame to position dPoint.
        m_frameset->moveFloatingFrame( m_frameNum, dPoint );
    } else
    {
        // This can happen if the page hasn't been created yet
        kDebug(32001) <<"KWAnchor::move internalToDocument returned 0L for" << x()+paragx <<"," << y()+paragy;
    }
#endif
}

void KWAnchor::draw( QPainter* p, int x, int y, int cx, int cy, int cw, int ch, const QColorGroup& cg, bool selected )
{
#if 0
    // (x,y) is the position of the inline item (in Layout Units)
    // (cx,cy,cw,ch) is the rectangle to be painted, in layout units too

    if ( m_deleted )
        return;

    Q_ASSERT( x == xpos );
    Q_ASSERT( y == ypos );
    if ( x != xpos || y != ypos )
        kDebug() <<"Warning: x=" << x <<" y=" << y <<" xpos=" << xpos <<" ypos=" << ypos;

    // The containing text-frameset.
    KWTextFrameSet * fs = static_cast<KWTextDocument *>(textDocument())->textFrameSet();
    KoTextZoomHandler* zh = fs->textDocument()->paintingZoomHandler();

    int paragx = paragraph()->rect().x();
    int paragy = paragraph()->rect().y();
    QRect inlineFrameLU( paragx+xpos, paragy+ypos, width, height );
#ifdef DEBUG_DRAWING
    kDebug(32001) <<"KWAnchor::draw x:" << x <<", y:" << y <<" paragx=" << paragx <<" paragy=" << paragy;
    kDebug(32001) <<"               inline frame in LU coordinates:" << inlineFrameLU;
#endif

    QRect crectLU = QRect( (cx > 0 ? cx : 0)+paragx, cy+paragy, cw, ch );
#ifdef DEBUG_DRAWING
    kDebug(32001) <<"               crect in LU coordinates:" << DEBUGRECT( crectLU );
#endif

    crectLU = crectLU.intersect ( inlineFrameLU ); // KoTextParag::paintDefault could even do this


#ifdef DEBUG_DRAWING
    kDebug(32001) <<"               crect&frame in LU coordinates:" << DEBUGRECT( crectLU );
#endif

    // Convert crect to document coordinates, first topleft then bottomright
    QPoint topLeftLU = crectLU.topLeft();
    QPoint bottomRightLU = crectLU.bottomRight();
    KWFrame* containingFrame = fs->currentDrawnFrame(); // always set, except in the textviewmode
    if(containingFrame)
        containingFrame = KWFrameSet::settingsFrame(containingFrame);
    else { // if its not set (in textviewmode) try to get it from the FS
        QPoint paragPos = inlineFrameLU.topLeft();
        KoPoint dummy(0, 0);
        containingFrame = fs->internalToDocument(paragPos, dummy);
    }
    KoPoint topLeftPt = fs->internalToDocumentKnowingFrame( topLeftLU, containingFrame );

    // Now we can convert the bottomright
    KoPoint bottomRightPt = fs->internalToDocumentKnowingFrame( bottomRightLU, containingFrame );
    KoRect crectPt( topLeftPt, bottomRightPt );

    // Convert crect to view coords
    QRect crect = fs->currentViewMode()->normalToView( zh->zoomRectOld( crectPt ) );
    // and add 1 to right and bottom, to avoid rounding errors (and due to qrect semantics)
    crect.rBottom() += 2; // HACK: 1 doesn't do it, it leaves a white line along window borders
    crect.rRight() += 1;
#ifdef DEBUG_DRAWING
    kDebug() <<"               crect in view coordinates (pixel) :" << DEBUGRECT( crect );
#endif

    // Ok, we finally have our crect in view coordinates!
    // Now ensure the containing frame is the one actually containing our text
    // (for copies, e.g. headers and footers, we need to go back until finding a real frame)

    if ( containingFrame && containingFrame->isCopy() )
    {
        // Find last real frame, in case we are in a copied frame
        Q3PtrListIterator<KWFrame> frameIt( fs->frameIterator() );
        frameIt.toLast(); // from the end to avoid a 2*N in the worst case
        while ( !frameIt.atFirst() && frameIt.current() != containingFrame ) // look for 'containingFrame'
            --frameIt;
        if ( frameIt.atFirst() && frameIt.current() != containingFrame )
            kWarning() << "KWAnchor::draw: containingFrame not found " << containingFrame << endl;
        while ( !frameIt.atFirst() && frameIt.current()->isCopy() ) // go back to last non-copy
            --frameIt;
        containingFrame = frameIt.current();
        //kDebug() <<"KWAnchor::draw frame=" << containingFrame;
    }

    // Same calculation as in internalToDocument, but we know the frame already
    QPointF topLeftParagPt( 0, 0 );
    if ( containingFrame ) // 0 in the textviewmode
        topLeftParagPt = containingFrame->innerRect().topLeft();

    topLeftParagPt.rx() += zh->layoutUnitPtToPt( zh->pixelYToPt( paragx ) );
    topLeftParagPt.ry() += zh->layoutUnitPtToPt( zh->pixelYToPt( paragy ) );
    if ( containingFrame ) // 0 in the textviewmode
        topLeftParagPt.ry() -= containingFrame->internalY();

    QPoint topLeftParag = fs->currentViewMode()->normalToView( zh->normalToView( topLeftParagPt ).toPoint() );

    // Finally, make the painter go back to view coord system
    // (this is exactly the opposite of the code in KWFrameSet::drawContents)
    // (It does translate(view - internal), so we do translate(internal - view) - e.g. with (0,0) for internal)
    p->save();
    p->translate( -topLeftParag.x(), -topLeftParag.y() );
#ifdef DEBUG_DRAWING
    kDebug() <<"               translating by" << -topLeftParag.x() <<"," << -topLeftParag.y();
#endif

    QColorGroup cg2( cg );

    KWFrameViewManager *fvm = 0;
    if(m_frameset->kWordDocument()) {
        QList<KWView *> views = m_frameset->kWordDocument()->getAllViews();
        // Note that "views" is empty when the KWDocument is an (inactive) embedded document
        if ( !views.isEmpty() )
            fvm = views.first()->frameViewManager();
    }
    m_frameset->drawContents( p, crect, cg2, false, true, 0L, fs->currentViewMode(), fvm);

    if( selected && placement() == PlaceInline && p->device()->devType() != QInternal::Printer )
    {
        // The above rects are about the containing frame.
        // To draw the inline frame as selected, we need to look at the inline frame's own size.
        QRect frameRect = crect;
#ifdef DEBUG_DRAWING
        kDebug() <<"KWAnchor::draw selected frame. frameRect=" << frameRect;
#endif
        p->fillRect( frameRect, QBrush( cg.highlight(), Qt::Dense4Pattern) );
    }
    p->restore();

#ifdef DEBUG_DRAWING
    kDebug() <<"KWAnchor::draw done";
#endif
#endif
}

QSize KWAnchor::size() const
{
    KoSize kosz = m_frameset->floatingFrameSize( m_frameNum );
    //kDebug() <<"KWAnchor::size in pt:" << kosz.width() <<"x" << kosz.height();
    KoTextZoomHandler * zh = textDocument()->formattingZoomHandler();
    QSize sz( zh->ptToLayoutUnitPixX( kosz.width() ), zh->ptToLayoutUnitPixY( kosz.height() ) );
    //kDebug() <<"KWAnchor::size in LU:" << sz.width() <<"x" << sz.height();
    //kDebug() <<"          size in pixels:" << zh->layoutUnitToPixelX( sz.width() ) <<"x"
    //          << zh->layoutUnitToPixelY( sz.height() ) << endl;
    if ( sz.isNull() ) // for some reason, we don't know the size yet
        sz = QSize( width, height ); // LU
    return sz;
}

int KWAnchor::ascent() const
{
    int baseline = m_frameset->floatingFrameBaseline( m_frameNum );
    int ret = ( baseline == -1 ) ? height : baseline;
    //kDebug() <<"KWAnchor::ascent" << ret;
    return ret;
}

void KWAnchor::resize()
{
    if ( m_deleted )
        return;
    QSize s = size();
    if ( width != s.width() || height != s.height() )
    {
        width = s.width();
        height = s.height();
        kDebug(32001) <<"KWAnchor::resize" << width <<"x" << height;
        KoTextParag * parag = paragraph();
        if ( parag )
        {
            kDebug(32001) <<"KWAnchor::resize invalidating parag" << parag->paragId();
            parag->invalidate( 0 );
        }
    }
}

KCommand * KWAnchor::createCommand()
{
    kDebug(32001) <<"KWAnchor::addCreateCommand";
    return m_frameset->anchoredObjectCreateCommand( m_frameNum );
}

KCommand * KWAnchor::deleteCommand()
{
    kDebug(32001) <<"KWAnchor::addDeleteCommand";
    return m_frameset->anchoredObjectDeleteCommand( m_frameNum );
}

void KWAnchor::setDeleted( bool b )
{
    // Do this first, because setAnchored->updateAllFrames->isDeleted, so it must have the right value already
    KoTextCustomItem::setDeleted( b );

    kDebug() <<"KWAnchor::setDeleted" << b;
    if ( b )
        m_frameset->setAnchored( 0L );
    else
        m_frameset->setAnchored( static_cast<KWTextDocument *>(textDocument())->textFrameSet() );
}

void KWAnchor::save( QDomElement &parentElem )
{
    QDomElement anchorElem = parentElem.ownerDocument().createElement( "ANCHOR" );
    parentElem.appendChild( anchorElem );
    anchorElem.setAttribute( "type", "frameset" ); // the only possible value currently
    //KWDocument * doc = textDocument()->textFrameSet()->kWordDocument();
    // ## TODO save the frame number as well ? Only the first frame ? to be determined
    // ## or maybe use len=<number of frames>. Difficult :}
    anchorElem.setAttribute( "instance", m_frameset->name() );
}

void KWAnchor::saveOasis( KoXmlWriter& writer, KoTextSavingContext& context ) const
{
    if ( m_frameset->canBeSavedAsInlineCharacter() )
        m_frameset->saveOasis( writer, context, true );
    else // special case for inline tables [which are not alone in their paragraph, see KWTextParag]
    {
        writer.startElement( "draw:frame" );
        writer.addAttribute( "draw:name", m_frameset->name() + "-Wrapper" );
        // Mark as wrapper frame. KWTextDocument::loadSpanTag will try to get rid of it upon loading.
        writer.addAttribute( "koffice:is-wrapper-frame", "true" );
        //writer.addAttribute( "draw:style-name", saveOasisFrameStyle( mainStyles ) );
        KoSize kosz = m_frameset->floatingFrameSize( m_frameNum );
        writer.addAttributePt( "svg:width", kosz.width() );
        writer.addAttributePt( "svg:height", kosz.height() );
        writer.startElement( "draw:text-box" );
        m_frameset->saveOasis( writer, context, true );
        writer.endElement();
        writer.endElement();
    }
}

bool KWAnchor::ownLine() const
{
    if ( m_deleted )
        return false;

    if ( m_frameset)
        return m_frameset->ownLine();
    return false;
}
