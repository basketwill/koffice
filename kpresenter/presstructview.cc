/******************************************************************/
/* KPresenter - (c) by Reginald Stadlbauer 1997-1998              */
/* Version: 0.1.0                                                 */
/* Author: Reginald Stadlbauer                                    */
/* E-Mail: reggie@kde.org                                         */
/* Homepage: http://boch35.kfunigraz.ac.at/~rs                    */
/* needs c++ library Qt (http://www.troll.no)                     */
/* written for KDE (http://www.kde.org)                           */
/* needs mico (http://diamant.vsb.cs.uni-frankfurt.de/~mico/)     */
/* needs OpenParts and Kom (weis@kde.org)                         */
/* License: GNU GPL                                               */
/******************************************************************/
/* Module: Presentation Structure Viewer (header)                 */
/******************************************************************/

#include "presstructview.h"
#include "presstructview.moc"

#include "kpbackground.h"
#include "kpobject.h"
#include "kpresenter_doc.h"

#include <klocale.h>
#include <kiconloader.h>

#include <qsplitter.h>
#include <qevent.h>
#include <qheader.h>

/******************************************************************
 *
 * Class: KPPresStructObjectItem
 *
 ******************************************************************/

/*================================================================*/
KPPresStructObjectItem::KPPresStructObjectItem( QListView *parent )
    : QListViewItem( parent ), page( 0 ), object( 0 )
{
}
    
/*================================================================*/
KPPresStructObjectItem::KPPresStructObjectItem( QListViewItem *parent )
    : QListViewItem( parent ), page( 0 ), object( 0 )
{
}

/*================================================================*/
void KPPresStructObjectItem::setPage( KPBackGround *p )
{
    page = p;
    if ( page && !parent() )
	setPixmap( 0, BarIcon( "dot" ) );
}

/*================================================================*/
void KPPresStructObjectItem::setObject( KPObject *o )
{
    object = o;
    if ( object && parent() )
	;
}

/*================================================================*/
KPBackGround *KPPresStructObjectItem::getPage()
{
    return page;
}

/*================================================================*/
KPObject *KPPresStructObjectItem::getObject()
{
    return object;
}

/******************************************************************
 *
 * Class: KPPresStructView
 *
 ******************************************************************/

/*================================================================*/
KPPresStructView::KPPresStructView( QWidget *parent, const char *name,
                                    KPresenterDoc *_doc, KPresenterView *_view )
    : QDialog( parent, name, FALSE ), doc( _doc ), view( _view )
{
    hsplit = new QSplitter( this );
    setupSlideList();
    
    resize( 600, 400 );
}

/*================================================================*/
void KPPresStructView::setupSlideList()
{
    slides = new QListView( hsplit );
    slides->addColumn( i18n( "Slide Nr." ) );
    slides->addColumn( i18n( "Slide Title" ) );
    slides->header()->setMovingEnabled( FALSE );
    slides->setAllColumnsShowFocus( TRUE );
    slides->setRootIsDecorated( TRUE );
    slides->setSorting( -1 );
    
    for ( int i = doc->getPageNums() - 1; i >= 0; --i ) {
        KPPresStructObjectItem *item = new KPPresStructObjectItem( slides );
	item->setPage( doc->backgroundList()->at( i ) );
        item->setText( 0, QString( "%1" ).arg( i + 1 ) );
        item->setText( 1, doc->getPageTitle( i, i18n( "Slide %1" ).arg( i + 1 ) ) );
	for ( int j = doc->objNums() - 1; j >= 0; --j ) {
	    if ( doc->getPageOfObj( j, 0, 0 ) == (int)i + 1 ) {
		KPPresStructObjectItem *item_ = new KPPresStructObjectItem( item );
		item_->setPage( doc->backgroundList()->at( i ) );
		item_->setObject( doc->objectList()->at( j ) );
	    }
	}
    }
}

/*================================================================*/
void KPPresStructView::resizeEvent( QResizeEvent *e )
{
    QDialog::resizeEvent( e );
    hsplit->resize( size() );
}
