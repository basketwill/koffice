/* This file is part of the KDE project
   Copyright (C) 2003 Ariya Hidayat <ariya@kde.org>
   Copyright (C) 2003 Norbert Andres <nandres@web.de>
   Copyright (C) 2002 Laurent Montel <montel@kde.org>
   Copyright (C) 1999 David Faure <faure@kde.org>
   Copyright (C) 1999 Boris Wedl <boris.wedl@kfunigraz.ac.at>
   Copyright (C) 1998-2000 Torben Weis <weis@kde.org>

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

#include "kspread_tabbar.h"

#include <qdrawutil.h>
#include <qpainter.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtimer.h>
#include <qtoolbutton.h>
#include <qvaluevector.h>
#include <qwidget.h>

// TODO
// improvement possibilities
// - use offscreen buffer to reduce flicker even more
// - keep track of tabs, only (re)layout when necessary
// - paint all tabs to buffer, show only by shifting
// - customizable button pixmaps
// - use QStyle to paint the tabs & buttons (is it good/possible?)


namespace KSpread
{

class TabBarPrivate
{
public:
    TabBar* tabbar;

    // scroll buttons
    QToolButton* scrollFirstButton;
    QToolButton* scrollLastButton;
    QToolButton* scrollBackButton;
    QToolButton* scrollForwardButton;

    // read-only: no mouse drag, double-click, right-click
    bool readOnly;
    
    // if true, layout is from right to left
    bool reverseLayout;

    // list of all tabs, in order of appearance
    QStringList tabs;

    // array of QRect for each visible tabs
    QValueVector<QRect> tabRects;

    // leftmost tab (or rightmost if reverseLayout)
    int firstTab;
    
    // rightmost tab (or leftmost if reverseLayout)
    int lastTab;

    // the active tab in the range form 1..n.
    // if this value is 0, that means that no tab is active.
    int activeTab;

    // unusable space on the left, taken by the scroll buttons
    int offset;

    // when the user drag the tab (in order to move it)
    // this is the target position, it's 0 if no tab is dragged
    int targetTab;

    // true if autoscroll is active
    bool autoScroll;

    // calculate the bounding rectangle for each visible tab
    void layoutTabs();
    
    // reposition scroll buttons
    void layoutButtons();

    // find a tab whose bounding rectangle contains the pos
    // return -1 if no such tab is found
    int tabAt( const QPoint& pos );

    // draw a single tab
    void drawTab( QPainter& painter, QRect& rect, const QString& text, bool active );

    // draw a marker to indicate tab moving
    void drawMoveMarker( QPainter& painter, int x, int y );

    // update the enable/disable status of scroll buttons
    void updateButtons();
};


};

using namespace KSpread;

// built-in pixmap for scroll-first button
static const char * arrow_leftmost_xpm[] = {
"10 10 2 1",
" 	c None",
".	c #000000",
"          ",
"  .    .  ",
"  .   ..  ",
"  .  ...  ",
"  . ....  ",
"  .  ...  ",
"  .   ..  ",
"  .    .  ",
"          ",
"          "};

// built-in pixmap for scroll-last button
static const char * arrow_rightmost_xpm[] = {
"10 10 2 1",
" 	c None",
".	c #000000",
"          ",
"  .    .  ",
"  ..   .  ",
"  ...  .  ",
"  .... .  ",
"  ...  .  ",
"  ..   .  ",
"  .    .  ",
"          ",
"          "};

// built-in pixmap for scroll-left button
static const char * arrow_left_xpm[] = {
"10 10 2 1",
" 	c None",
".	c #000000",
"          ",
"      .   ",
"     ..   ",
"    ...   ",
"   ....   ",
"    ...   ",
"     ..   ",
"      .   ",
"          ",
"          "};

// built-in pixmap for scroll-right button
static const char * arrow_right_xpm[] = {
"10 10 2 1",
" 	c None",
".	c #000000",
"          ",
"   .      ",
"   ..     ",
"   ...    ",
"   ....   ",
"   ...    ",
"   ..     ",
"   .      ",
"          ",
"          "};


void TabBarPrivate::layoutTabs()
{
    tabRects.clear();

    QPainter painter( tabbar );

    QFont f = painter.font();
    f.setBold( true );
    painter.setFont( f );
    QFontMetrics fm = painter.fontMetrics();
    
    if( !reverseLayout )
    {
        // left to right
        int x = 0;
        for( unsigned c = 0; c < tabs.count(); c++ )
        {
            QRect rect;
            if( (int)c >= firstTab-1 )
            {
                QString text = tabs[ c ];
                int tw = fm.width( text ) + 4;
                rect = QRect( x, 0, tw + 20, tabbar->height() );
                x = x + tw + 10;
            }
            tabRects.append( rect );
        }

        lastTab = tabRects.count();
        for( unsigned i = 0; i < tabRects.count(); i++ )
            if( tabRects[i].right()-10+offset > tabbar->width() )
            {
                lastTab = i;
                break;
            }
    }
    else
    {
        // right to left
        int x = tabbar->width() - offset;
        for( unsigned c = 0; c < tabs.count(); c++ )
        {
            QRect rect;
            if( (int)c >= firstTab-1 )
            {
                QString text = tabs[ c ];
                int tw = fm.width( text ) + 4;
                rect = QRect( x - tw - 20, 0, tw + 20, tabbar->height() );
                x = x - tw - 10;
            }
            tabRects.append( rect );
        }

        lastTab = tabRects.count();
        for( unsigned i = 0; i < tabRects.count(); i++ )
            if( tabRects[i].right() < 0 )
            {
                lastTab = i;
                break;
            }
    }    
}

int TabBarPrivate::tabAt( const QPoint& pos )
{
    for( unsigned i = 0; i < tabRects.count(); i++ )
    {
      QRect rect = tabRects[ i ];
      if( rect.isNull() ) continue;
      if( rect.contains( pos ) ) return i;
    }

    return -1; // not found
}

void TabBarPrivate::drawTab( QPainter& painter, QRect& rect, const QString& text, bool active )
{
    QPointArray pa;
    pa.setPoints( 4, rect.x(), rect.y(), rect.x()+10, rect.bottom()-1,
      rect.right()-10, rect.bottom()-1, rect.right(), rect.top() );

    QColor bgcolor = tabbar->colorGroup().background();
    if( active ) bgcolor = tabbar->colorGroup().base();

    painter.setClipping( true );
    painter.setClipRegion( QRegion( pa ) );
    painter.setBackgroundColor( bgcolor );
    painter.eraseRect( rect );
    painter.setClipping( false );

    painter.drawLine( rect.x(), rect.y(), rect.x()+10, rect.bottom()-1 );
    painter.drawLine( rect.x()+10, rect.bottom()-1, rect.right()-10, rect.bottom()-1 );
    painter.drawLine( rect.right()-10, rect.bottom()-1, rect.right(), rect.top() );
    if( !active )
      painter.drawLine( rect.x(), rect.y(), rect.right(), rect.y() );

    painter.save();
    QFont f = painter.font();
    if( active ) f.setBold( true );
    painter.setFont( f );
    QFontMetrics fm = painter.fontMetrics();
    int tx =  rect.x() + ( rect.width() - fm.width( text ) ) / 2;
    int ty =  rect.y() + ( rect.height() - fm.ascent() - fm.descent() ) / 2 + fm.ascent();
    painter.drawText( tx, ty, text );
    painter.restore();
}

void TabBarPrivate::drawMoveMarker( QPainter& painter, int x, int y )
{
    QPointArray movmark;
    movmark.setPoints( 3, x, y, x + 7, y, x + 4, y + 6);
    QBrush oldBrush = painter.brush();
    painter.setBrush( Qt::black );
    painter.drawPolygon(movmark);
    painter.setBrush( oldBrush );
}

void TabBarPrivate::layoutButtons()
{
    int bw = tabbar->height();
    int w = tabbar->width();
    offset = bw * 4;
    
    if( !reverseLayout )
    {
        scrollFirstButton->setGeometry( 0, 0, bw, bw );
        scrollFirstButton->setPixmap( arrow_leftmost_xpm );
        scrollBackButton->setGeometry( bw, 0, bw, bw );
        scrollBackButton->setPixmap( arrow_left_xpm );
        scrollForwardButton->setGeometry( bw*2, 0, bw, bw );
        scrollForwardButton->setPixmap( arrow_right_xpm );
        scrollLastButton->setGeometry( bw*3, 0, bw, bw );
        scrollLastButton->setPixmap( arrow_rightmost_xpm );
    }
    else
    {
        scrollFirstButton->setGeometry( w-bw, 0, bw, bw );
        scrollFirstButton->setPixmap( arrow_rightmost_xpm );
        scrollBackButton->setGeometry( w-2*bw, 0, bw, bw );
        scrollBackButton->setPixmap( arrow_right_xpm );
        scrollForwardButton->setGeometry( w-3*bw, 0, bw, bw );
        scrollForwardButton->setPixmap( arrow_left_xpm );
        scrollLastButton->setGeometry( w-4*bw, 0, bw, bw );
        scrollLastButton->setPixmap( arrow_leftmost_xpm );
    }
 }

void TabBarPrivate::updateButtons()
{
    scrollFirstButton->setEnabled( tabbar->canScrollBack() );
    scrollBackButton->setEnabled( tabbar->canScrollBack() );
    scrollForwardButton->setEnabled( tabbar->canScrollForward() );
    scrollLastButton->setEnabled( tabbar->canScrollForward() );
}

// creates a new tabbar
TabBar::TabBar( QWidget* parent, const char* name )
    : QWidget( parent, name, Qt::WResizeNoErase | Qt::WRepaintNoErase )
{
    d = new TabBarPrivate;
    d->tabbar = this;
    d->readOnly = false;
    d->reverseLayout = false;
    d->firstTab = 1;
    d->lastTab = 0;
    d->activeTab = 0;
    d->targetTab = 0;
    d->autoScroll = false;
    d->offset = 64;

    // initialize the scroll buttons
    d->scrollFirstButton = new QToolButton( this );
    connect( d->scrollFirstButton, SIGNAL( clicked() ),
      this, SLOT( scrollFirst() ) );
    d->scrollLastButton = new QToolButton( this );
    connect( d->scrollLastButton, SIGNAL( clicked() ),
      this, SLOT( scrollLast() ) );
    d->scrollBackButton = new QToolButton( this );
    connect( d->scrollBackButton, SIGNAL( clicked() ),
      this, SLOT( scrollBack() ) );
    d->scrollForwardButton = new QToolButton( this );
    connect( d->scrollForwardButton, SIGNAL( clicked() ),
      this, SLOT( scrollForward() ) );
    d->layoutButtons();
    d->updateButtons();
}

// destroys the tabbar
TabBar::~TabBar()
{
    delete d;
}

// adds a new visible tab
void TabBar::addTab( const QString& text )
{
    d->tabs.append( text );

    update();
}

// removes a tab
void TabBar::removeTab( const QString& text )
{
    int i = d->tabs.findIndex( text );
    if ( i == -1 ) return;

    if ( d->activeTab == i + 1 )
        d->activeTab = 0;

    d->tabs.remove( text );

    update();
}

// removes all tabs
void TabBar::clear()
{
    d->tabs.clear();
    d->activeTab = 0;
    d->firstTab = 1;

    update();
}

bool TabBar::readOnly() const
{
    return d->readOnly;
}

void TabBar::setReadOnly( bool ro )
{
    d->readOnly = ro;
}

bool TabBar::reverseLayout() const
{
    return d->reverseLayout;
}

void TabBar::setReverseLayout( bool reverse )
{
    if( reverse != d->reverseLayout )
    {
        d->reverseLayout = reverse;
        d->layoutTabs();
        d->layoutButtons();
        d->updateButtons();
        update();
    }
}

void TabBar::setTabs( const QStringList& list )
{
    QString left, active;

    if( d->activeTab > 0 )
        active = d->tabs[ d->activeTab-1 ];
    if( d->firstTab > 0 )
        left = d->tabs[ d->firstTab-1 ];

    d->tabs = list;

    if( !left.isNull() )
    {
        d->firstTab = d->tabs.findIndex( left ) + 1;
        if( d->firstTab > (int)d->tabs.count() )
            d->firstTab = 1;
        if( d->firstTab <= 0 )
            d->firstTab = 1;
    }

    d->activeTab = 0;
    if( !active.isNull() )
        setActiveTab( active );

    update();
}

QStringList TabBar::tabs() const
{
    return d->tabs;
}

unsigned TabBar::count() const
{
    return d->tabs.count();
}

bool TabBar::canScrollBack() const
{
    if ( d->tabs.count() == 0 )
        return false;
	
    return d->firstTab > 1;
}

bool TabBar::canScrollForward() const
{
    if ( d->tabs.count() == 0 )
        return false;

    return d->lastTab < d->tabs.count();
}

void TabBar::scrollBack()
{
    if ( !canScrollBack() )
        return;

    d->firstTab--;
    if( d->firstTab < 1 ) d->firstTab = 1;

    d->layoutTabs();
    d->updateButtons();
    update();
}

void TabBar::scrollForward()
{
    if ( !canScrollForward() )
        return;

    d->firstTab ++;
    if( d->firstTab > d->tabs.count() )
        d->firstTab = d->tabs.count();

    d->layoutTabs();
    d->updateButtons();
    update();
}

void TabBar::scrollFirst()
{
    if ( !canScrollBack() )
        return;

    d->firstTab = 1;
    d->layoutTabs();
    d->updateButtons();
    update();
}

void TabBar::scrollLast()
{
    if ( !canScrollForward() )
        return;

    d->layoutTabs();

    if( !d->reverseLayout )
    {
        int fullWidth = d->tabRects[ d->tabRects.count()-1 ].right();
        int delta = fullWidth - width() + d->offset;
        for( unsigned i = 0; i < d->tabRects.count(); i++ )
            if( d->tabRects[i].x() > delta )
            {
                d->firstTab = i+1;
                break;
            }
    }
    else
    {
        // FIXME optimize this, perhaps without loop
        for( ; d->firstTab <= d->tabRects.count();)
        {
            int x = d->tabRects[ d->tabRects.count()-1 ].x();
            if( x > 0 ) break;
            d->firstTab++;
            d->layoutTabs();
        }
    }

    d->layoutTabs();
    d->updateButtons();
    update();
}

void TabBar::moveTab( unsigned tab, unsigned target )
{
    QString tabName = d->tabs[ tab ];
    QStringList::Iterator it;

    it = d->tabs.at( tab );
    d->tabs.remove( it );

    if( target > tab ) target--;
    it = d->tabs.at( target );
    if( target >= d->tabs.count() )
      it = d->tabs.end();
    d->tabs.insert( it, tabName );

    if( d->activeTab == (int)tab+1 )
        d->activeTab = target+1;

    update();
}

void TabBar::setActiveTab( const QString& text )
{
    int i = d->tabs.findIndex( text );
    if ( i == -1 )
        return;

    if ( i + 1 == d->activeTab )
        return;

    d->activeTab = i + 1;
    d->updateButtons();
    update();

    emit tabChanged( text );
}

void TabBar::autoScrollBack()
{
    if( !d->autoScroll ) return;

    scrollBack();

    if( !canScrollBack() )
        d->autoScroll = false;
    else
        QTimer::singleShot( 400, this, SLOT( autoScrollBack() ) );
}

void TabBar::autoScrollForward()
{
    if( !d->autoScroll ) return;

    scrollForward();

    if( !canScrollForward() )
        d->autoScroll = false;
    else
        QTimer::singleShot( 400, this, SLOT( autoScrollForward() ) );
}

void TabBar::paintEvent( QPaintEvent* )
{
    if ( d->tabs.count() == 0 )
    {
        erase();
        return;
    }

    QPainter painter;
    QPixmap pm( size() );
    pm.fill( colorGroup().background() );
    painter.begin( &pm, this );

    QBrush fill( colorGroup().brush( QColorGroup::Background ) );
    qDrawShadePanel( &painter, 0, 0, width(),
                     height(), colorGroup(), FALSE, 1, &fill );

    d->layoutTabs();
    
    // draw first all non-active, visible tabs
    for( unsigned c = 0; c < d->tabRects.count(); c++ )
    {
        QRect rect = d->tabRects[ c ];
        if( rect.isNull() ) continue;
        QString text = d->tabs[ c ];
        d->drawTab( painter, rect, text, false );
    }

    // draw the active tab
    if( d->activeTab > 0 )
    {
        QRect rect = d->tabRects[ d->activeTab-1 ];
        if( !rect.isNull() )
        {
            QString text = d->tabs[ d->activeTab-1 ];
            d->drawTab( painter, rect, text, true );
        }
    }

    // draw the move marker
    if( d->targetTab > 0 )
    {
        int p = QMIN( d->targetTab, (int)d->tabRects.count() );
        QRect rect = d->tabRects[ p-1 ];
        if( !rect.isNull() )
        {
            int x = rect.x();
            if( d->targetTab > (int)d->tabRects.count() )
              x = rect.right()-7;
            d->drawMoveMarker( painter, x, rect.y() );
        }
    }

    painter.end();
    
    if( !d->reverseLayout )
         bitBlt( this, d->offset, 0, &pm );
    else
         bitBlt( this, 0, 0, &pm );

}

void TabBar::resizeEvent( QResizeEvent* )
{
    d->layoutButtons();
    d->updateButtons();
    update();
}

void TabBar::renameTab( const QString& old_name, const QString& new_name )
{
    QStringList::Iterator it = d->tabs.find( old_name );
    (*it) = new_name;

    update();
}

QString TabBar::activeTab() const
{
    if( d->activeTab == 0 )
        return QString::null;
    else
        return d->tabs[ d->activeTab ];
}

void TabBar::mousePressEvent( QMouseEvent* ev )
{
    if ( d->tabs.count() == 0 )
    {
        erase();
        return;
    }

    d->layoutTabs();

    QPoint pos = ev->pos();
    if( !d->reverseLayout ) pos = pos - QPoint( d->offset,0 );
    
    int tab = d->tabAt( pos ) + 1;
    if( ( tab > 0 ) && ( tab != d->activeTab ) )
    {
        d->activeTab = tab;
        update();

        emit tabChanged( d->tabs[ d->activeTab-1] );

        // scroll if partially visible
        if( d->tabRects[ tab-1 ].right() > width() - d->offset )
            scrollForward();
    }

    if( ev->button() == RightButton )
    if( !d->readOnly )
        emit contextMenu( ev->globalPos() );
}

void TabBar::mouseReleaseEvent( QMouseEvent* ev )
{
    if ( d->readOnly ) return;

    d->autoScroll = false;

    if ( ev->button() == LeftButton && d->targetTab != 0 )
    {
        emit tabMoved( d->activeTab-1, d->targetTab-1 );
        d->targetTab = 0;
    }
}

void TabBar::mouseMoveEvent( QMouseEvent* ev )
{
    if ( d->readOnly ) return;

    QPoint pos = ev->pos();
    if( !d->reverseLayout) pos = pos - QPoint( d->offset,0 );
    
    // check if user drags a tab to move it
    int i = d->tabAt( pos ) + 1;
    if( ( i > 0 ) && ( i != d->targetTab ) )
    {
        if( i == d->activeTab ) i = 0;
        if( i == d->activeTab+1 ) i = 0;

        if( i != d->targetTab )
        {
           d->targetTab = i;
           d->autoScroll = false;
           update();
        }
    }

    // drag past the very latest visible tab
    // e.g move a tab to the last ordering position
    QRect r = d->tabRects[ d->tabRects.count()-1 ];
    if( r.isValid() )
    if( pos.x() > r.right() )
    if( pos.x() < width() )
    if( d->targetTab != (int)d->tabRects.count()+1 )
    {
        d->targetTab = d->tabRects.count()+1;
        d->autoScroll = false;
        update();
    }

    // outside far too left ? activate autoscroll...
    if ( pos.x() < 0 && !d->autoScroll  )
    {
        d->autoScroll = true;
        autoScrollBack();
    }

    // outside far too right ? activate autoscroll...
    int w = width() - d->offset;
    if ( pos.x() > w && !d->autoScroll )
    {
        d->autoScroll = true;
        autoScrollForward();
    }
}

void TabBar::mouseDoubleClickEvent( QMouseEvent* ev )
{
    int offset = d->reverseLayout ? 0 : d->offset;
    if( ev->pos().x() > offset )
    if( !d->readOnly )
        emit doubleClicked();
}


#include "kspread_tabbar.moc"
