/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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

#include <koKoolBar.h>

#include <qpainter.h>
#include <qcolor.h>
#include <qscrollbar.h>

static int g_koKoolBarId = 0;

KoKoolBar::KoKoolBar( QWidget *_parent, const char *_name ) :
  QWidget( _parent, _name ), m_iActiveGroup( -1 ), m_pButtonUp( 0L ),
  m_pButtonDown( 0L )
{
  m_mapGroups.setAutoDelete( true );
  m_pBox = new KoKoolBarBox( this );
}

int KoKoolBar::insertGroup( const QString& _text )
{
  KoKoolBarGroup *p = new KoKoolBarGroup( this, _text );
  m_mapGroups.insert( p->id(), p );

  if ( m_iActiveGroup == -1 )
    setActiveGroup( p->id() );
  else
    resizeEvent( 0L );
  return p->id();
}

int KoKoolBar::insertItem( int _grp, const QPixmap& _pix, const QString& _text,
			   QObject *_obj, const char *_slot )
{
  KoKoolBarGroup* g = m_mapGroups[ _grp ];
  if ( !g )
    return -1;
  KoKoolBarItem *item = new KoKoolBarItem( g, _pix, _text );

  if ( _obj != 0L && _slot != 0L )
    connect( item, SIGNAL( pressed( int, int ) ), _obj, _slot );
  g->append( item );

  if ( g->id() == m_iActiveGroup )
    m_pBox->update();

  return item->id();
}

void KoKoolBar::removeGroup( int _grp )
{
  KoKoolBarGroup* g = m_mapGroups[ _grp ];
  if ( !g )
    return;

  m_mapGroups.remove( _grp );

  if ( _grp == m_iActiveGroup )
  {
    if ( m_mapGroups.count() == 0 )
    {
      m_iActiveGroup = -1;
      m_pBox->setActiveGroup( 0L );
    }
    else
    {
      QIntDictIterator<KoKoolBarGroup> it( m_mapGroups );
      g = it.current();
      m_iActiveGroup = g->id();
      m_pBox->setActiveGroup( g );
    }
  }

  resizeEvent( 0L );
}

void KoKoolBar::removeItem( int _grp, int _id )
{
  KoKoolBarGroup* g = m_mapGroups[ _grp ];
  if ( !g )
    return;

  g->remove( _id );

  if ( g->id() == m_iActiveGroup )
    m_pBox->update();
}

void KoKoolBar::renameItem( int _grp, int _id, const QString & _text )
{
  KoKoolBarGroup* g = m_mapGroups[ _grp ];
  if ( !g )
    return;

  KoKoolBarItem * item = g->item( _id );
  if ( !item )
    return;

  item->setText( _text );

  if ( g->id() == m_iActiveGroup )
    m_pBox->update();
}

void KoKoolBar::setActiveGroup( int _grp )
{
  KoKoolBarGroup* g = m_mapGroups[ _grp ];
  if ( !g )
    return;

  m_iActiveGroup = g->id();
  m_pBox->setActiveGroup( g );

  resizeEvent( 0L );
}

void KoKoolBar::resizeEvent( QResizeEvent * )
{
  if ( m_iActiveGroup == -1 )
    return;

  int buttonheight = fontMetrics().height() + 4;

  KoKoolBarGroup *g = m_mapGroups[ m_iActiveGroup ];
  if ( !g )
    return;

  // Go behind g
  QIntDictIterator<KoKoolBarGroup> it( m_mapGroups );
  while( it.current() != g )
    ++it;
  // Position of g
  QIntDictIterator<KoKoolBarGroup> pos = it;
  ++it;

  // How many left ?
  int result = 0;
  QIntDictIterator<KoKoolBarGroup> i = it;
  while( i.current() )
  {
    ++result;
    ++i;
  }

  int y = height() - buttonheight * result;
  for( ; it.current(); ++it )
  {
    it.current()->button()->setGeometry( 0, y, width(), buttonheight );
    it.current()->button()->show();
    y += buttonheight;
  }

  int y2 = 0;
  it.toFirst();
  ++pos;
  while( it.current() != pos.current() )
  {
    it.current()->button()->setGeometry( 0, y2, width(), buttonheight );
    it.current()->button()->show();
    ++it;
    y2 += buttonheight;
  }

  if ( height() - y2 - result * buttonheight >= 0 )
  {
    m_pBox->show();
    m_pBox->setGeometry( 0, y2, width(), height() - y2 - result * buttonheight );
  }
  else
    m_pBox->hide();

  if ( m_pBox->needsScrolling() )
  {
    if ( m_pButtonUp == 0L )
    {
      m_pButtonUp = new QPushButton( this );
      connect( m_pButtonUp, SIGNAL( clicked() ), this, SLOT( slotUp() ) );
    }
    if ( m_pButtonDown == 0L )
    {
      m_pButtonDown = new QPushButton( this );
      connect( m_pButtonDown, SIGNAL( clicked() ), this, SLOT( slotDown() ) );
    }
    m_pButtonUp->show();
    m_pButtonUp->raise();
    m_pButtonDown->show();
    m_pButtonDown->raise();
    m_pButtonUp->setGeometry( width() - 12, height() - result * buttonheight - 2 * 12, 12, 12 );
    m_pButtonDown->setGeometry( width() - 12, height() - result * buttonheight - 12, 12, 12 );
    updateScrollButtons();
  }
  else
  {
    if ( m_pButtonUp )
      m_pButtonUp->hide();
    if ( m_pButtonDown )
      m_pButtonDown->hide();
  }
}

void KoKoolBar::updateScrollButtons()
{
  if ( m_pBox->isAtTop() )
    m_pButtonUp->setEnabled( false );
  else
    m_pButtonUp->setEnabled( true );

  if ( m_pBox->isAtBottom() )
    m_pButtonDown->setEnabled( false );
  else
    m_pButtonDown->setEnabled( true );
}

void KoKoolBar::slotUp()
{
  m_pBox->scrollUp();
  updateScrollButtons();
}

void KoKoolBar::slotDown()
{
  m_pBox->scrollDown();
  updateScrollButtons();
}

void KoKoolBar::enableItem( int _grp, int _id, bool _enable )
{
  KoKoolBarGroup* g = m_mapGroups[ _grp ];
  if ( !g )
    return;
  KoKoolBarItem *item = g->item( _id );
  if ( !item )
    return;
  item->setEnabled( _enable );
}

void KoKoolBar::enableGroup( int _grp, bool _enable )
{
  KoKoolBarGroup* g = m_mapGroups[ _grp ];
  if ( !g )
    return;
  g->setEnabled( _enable );
}

KoKoolBarBox::KoKoolBarBox( KoKoolBar *_bar ) :
  QWidget( _bar ), m_pBar( _bar )
{
  m_iYOffset = 0;
  m_iYIcon = 0;
  m_pGroup = 0L;

  // setBackgroundMode( PaletteBase );
  setBackgroundColor( darkGray );
}

void KoKoolBarBox::setActiveGroup( KoKoolBarGroup *_grp )
{
  m_pGroup = _grp;
  m_iYOffset = 0;
  m_iYIcon = 0;
  update();
}

bool KoKoolBarBox::needsScrolling() const
{
  if ( m_pGroup == 0L )
    return false;

  int y = 0;

  QIntDictIterator<KoKoolBarItem> it = m_pGroup->iterator();
  for ( ; it.current(); ++it )
    y += it.current()->height();

  if ( y > height() )
    return true;

  /** TODO **/
  return false;
}

KoKoolBarItem* KoKoolBarBox::findByPos( int _abs_y ) const
{
  if ( m_pGroup == 0L )
    return 0L;

  int y = 0;

  QIntDictIterator<KoKoolBarItem> it = m_pGroup->iterator();
  for ( ; it.current(); ++it )
  {
    int dy = it.current()->height();
    if ( y <= _abs_y && _abs_y <= y + dy )
      return it.current();
    y += dy;
  }

  return 0L;
}

int KoKoolBarBox::maxHeight() const
{
  int y = 0;

  QIntDictIterator<KoKoolBarItem> it = m_pGroup->iterator();
  for ( ; it.current(); ++it )
    y += it.current()->height();

  return y;
}

bool KoKoolBarBox::isAtTop() const
{
  if ( m_iYIcon == 0 )
    return true;
  return false;
}

bool KoKoolBarBox::isAtBottom() const
{
  if ( m_pGroup->items() == 0 )
    return true;
  int h = maxHeight();
  if ( height() + m_iYOffset >= h )
    return true;
  if ( m_pGroup->items() - 1 == m_iYIcon )
    return true;
  return false;
}

void KoKoolBarBox::scrollUp()
{
  if ( isAtTop() )
    return;

  int y = 0;
  int i = 0;
  m_iYIcon--;

  QIntDictIterator<KoKoolBarItem> it = m_pGroup->iterator();
  for ( ; i < m_iYIcon && it.current(); ++it )
  {
    y += it.current()->height();
    ++i;
  }

  int old = m_iYOffset;
  m_iYOffset = y;

  QWidget::scroll( 0, old - m_iYOffset );
}

void KoKoolBarBox::scrollDown()
{
  if ( isAtBottom() )
    return;

  int y = 0;
  int i = 0;
  m_iYIcon++;

  QIntDictIterator<KoKoolBarItem> it = m_pGroup->iterator();
  for ( ; i < m_iYIcon && it.current(); ++it )
  {
    y += it.current()->height();
    i++;
  }

  int old = m_iYOffset;
  m_iYOffset = y;

  QWidget::scroll( 0, old - m_iYOffset );
}

void KoKoolBarBox::paintEvent( QPaintEvent * )
{
  QPainter painter;
  painter.begin( this );

  static QColorGroup g2( black, white, white, darkGray, lightGray, white, black );
  static QBrush fill2( darkGray );
  qDrawShadePanel( &painter, 0, 0, width(), height(), g2, true, 1, &fill2 );

  if ( m_pGroup == 0L )
  {
    painter.end();
    return;
  }

  int y = -m_iYOffset;

  QIntDictIterator<KoKoolBarItem> it = m_pGroup->iterator();
  for ( ; it.current(); ++it )
  {
    if ( y + it.current()->height() >= 0 && y <= height() )
      painter.drawPixmap( ( width() - it.current()->pixmap().width() ) / 2, y, it.current()->pixmap() );
    if ( it.current()->text() != 0L && it.current()->text()[0] != 0 )
    {
      int y2 = y;
      y2 += it.current()->pixmap().height() + 2;
      painter.drawText( ( width() - painter.fontMetrics().width( it.current()->text() ) ) / 2,
			y2 + painter.fontMetrics().ascent(), it.current()->text() );
    }

    y += it.current()->height();
  }

  painter.end();
}

KoKoolBarGroup::KoKoolBarGroup( KoKoolBar *_bar, const QString& _text ) :
  m_pBar( _bar )
{
  m_mapItems.setAutoDelete( true );

  m_pButton = new QPushButton( _text, _bar );

  m_bEnabled = true;

  connect( m_pButton, SIGNAL( clicked() ), this, SLOT( pressed() ) );
  m_id = g_koKoolBarId++;
}

void KoKoolBarGroup::remove( int _id )
{
  m_mapItems.remove( _id );
}

void KoKoolBarGroup::pressed()
{
  m_pBar->setActiveGroup( m_id );
}

KoKoolBarItem::KoKoolBarItem( KoKoolBarGroup *_grp, const QPixmap& _pix, const QString&_text )
  : m_pGroup( _grp )
{
  m_pixmap = _pix;
  m_strText = _text;
  m_bEnabled = true;
  m_id = g_koKoolBarId++;
  calc( _grp->bar() );
}

void KoKoolBarItem::calc( QWidget *_widget )
{
  m_iHeight = pixmap().height() + 8;

  if ( !m_strText.isEmpty() )
    m_iHeight += _widget->fontMetrics().height() + 2;
}

void KoKoolBarItem::press()
{
  emit pressed();
  emit pressed( m_pGroup->id(), m_id );
}

/*
#include <kapp.h>

int main( int argc, char **argv )
{
  KApplication app( argc, argv );
  KoKoolBar bar;
  int file = bar.insertGroup("File");
  QPixmap pix;
  pix.load( "/opt/kde/share/icons/image.xpm" );
  bar.insertItem( file, pix );
  pix.load( "/opt/kde/share/icons/html.xpm" );
  bar.insertItem( file, pix );
  pix.load( "/opt/kde/share/icons/txt.xpm" );
  bar.insertItem( file, pix );
  pix.load( "/opt/kde/share/icons/kfm.xpm" );
  bar.insertItem( file, pix );

  bar.insertGroup("Edit");
  bar.insertGroup("View");
  bar.insertGroup("Layout");
  bar.insertGroup("Help");
  bar.setGeometry( 100, 100, 80, 300 );
  bar.show();

  app.exec();
}
*/

#include <koKoolBar.moc>
