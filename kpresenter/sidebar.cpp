#include "sidebar.h"
#include "kpresenter_doc.h"
#include <qheader.h>
#include <qtimer.h>
#include <kdebug.h>

SideBar::SideBar( QWidget *parent, KPresenterDoc *d, KPresenterView *v )
    : KListView( parent ), doc( d ), view( v )
{
    rebuildItems();
    setSorting( -1 );
    header()->hide();
    addColumn( i18n( "Slide" ) );
    addColumn( i18n( "Number" ) );
    setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Expanding ) );
    connect( this, SIGNAL( currentChanged( QListViewItem * ) ), this, SLOT( itemClicked( QListViewItem * ) ) );
    connect( this, SIGNAL( moved( QListViewItem *, QListViewItem *, QListViewItem * ) ),
	     this, SLOT( movedItems( QListViewItem *, QListViewItem *, QListViewItem * ) ) );
    connect( this, SIGNAL( rightButtonPressed( QListViewItem *, const QPoint &, int ) ),
	     this, SLOT( rightButtonPressed( QListViewItem *, const QPoint &, int ) ) );
    setAcceptDrops( TRUE );
    setDropVisualizer( TRUE );
    setDragEnabled( TRUE );

    pageMenu = new QPopupMenu();
    CHECK_PTR( pageMenu );
    pageMenu->insertItem( i18n( "&Insert Page..." ), this, SLOT( pageInsert() ) );
    pageMenu->insertItem( i18n( "&Use current slide as default template" ), this, SLOT( pageDefaultTemplate() ) );
    pageMenu->insertItem( KPBarIcon( "newslide" ), i18n( "&Duplicate Page" ), this, SLOT( duplicateCopy() ) );
    delPageId = pageMenu->insertItem( KPBarIcon( "delslide" ), i18n( "D&elete Page..." ), this, SLOT( pageDelete() ) );
    pageMenu->setMouseTracking( true );
}

void SideBar::rebuildItems()
{
    // Map: page number -> checked
    QMap< int, bool > checkedMap;
    QListViewItemIterator it( this );
    for ( ; it.current(); ++it )
	checkedMap.insert( it.current()->text(1).toInt(), ( (QCheckListItem*)it.current() )->isOn() );

    clear();
    // Rebuild all the items
    for ( int i = doc->getPageNums() - 1; i >= 0; --i ) {
	QCheckListItem *item = new QCheckListItem( this, "", QCheckListItem::CheckBox );
	QString title = doc->getPageTitle( i, i18n( "Slide %1" ).arg( i + 1 ) );
	QMap< int, bool >::Iterator bit = checkedMap.find( i + 1 );
	item->setOn( bit != checkedMap.end() ? *bit : TRUE );
	item->setText( 1, QString::number( i + 1 ) ); // page number
	item->setText( 0, title );
    }
}

void SideBar::updateItem( int pagenr )
{
    // Find item
    QListViewItemIterator it( this );
    for ( ; it.current(); ++it )
    {
	if ( it.current()->text(1).toInt() == pagenr+1 )
        {
            QString title = doc->getPageTitle( pagenr, i18n( "Slide %1" ).arg( pagenr + 1 ) );
            it.current()->setText( 0, title );
            return;
        }
    }
    kdWarning() << "Item for page " << pagenr << " not found" << endl;
}

void SideBar::itemClicked( QListViewItem *i )
{
    if ( !i )
	return;
    emit showPage( i->text( 1 ).toInt() - 1 );
}


void SideBar::setCurrentPage( int pg )
{
    QListViewItemIterator it( this );
    for ( ; it.current(); ++it ) {
	if ( it.current()->text( 1 ).toInt() - 1 == pg ) {
	    setCurrentItem( it.current() );
	    setSelected( it.current(), TRUE );
	}
    }
}

void SideBar::setOn( int pg, bool on )
{
    QListViewItemIterator it( this );
    for ( ; it.current(); ++it ) {
	if ( it.current()->text( 1 ).toInt() - 1 == pg ) {
	    ( (QCheckListItem*)it.current() )->setOn( on );
	    return;
	}
    }
}

void SideBar::contentsDropEvent( QDropEvent *e )
{
    disconnect( this, SIGNAL( currentChanged( QListViewItem * ) ), this, SLOT( itemClicked( QListViewItem * ) ) );
    KListView::contentsDropEvent( e );
    connect( this, SIGNAL( currentChanged( QListViewItem * ) ), this, SLOT( itemClicked( QListViewItem * ) ) );
}

void SideBar::movedItems( QListViewItem *i, QListViewItem *, QListViewItem *newAfter )
{
    movedItem = i;
    movedAfter = newAfter;
    QTimer::singleShot( 300, this, SLOT( doMoveItems() ) );

}

void SideBar::doMoveItems()
{
    int num = movedItem->text( 1 ).toInt() - 1;
    int numNow;
    if ( !movedAfter ) {
	numNow = 0;
    } else {
	numNow = movedAfter->text( 1 ).toInt();
	if ( numNow > num )
	    numNow--;
    }
    emit movePage( num, numNow );
}

void SideBar::pageInsert()
{
    view->insertPage();
}

void SideBar::duplicateCopy()
{
    view->editDuplicatePage();
}

void SideBar::pageDelete()
{
    view->editDelPage();
}

void SideBar::pageDefaultTemplate()
{
    view->extraDefaultTemplate();
}

void SideBar::rightButtonPressed( QListViewItem *, const QPoint &pnt, int )
{
    if ( !selectedItem() )
        return;

    pageMenu->setItemEnabled( delPageId, doc->getPageNums() > 1 );
    pageMenu->popup( pnt );
}
