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

#include <fstream>

#include "koDocument.h"
#include "koDocumentChild.h"
#include "koView.h"
#include "koApplication.h"
#include "koMainWindow.h"
#include "koStream.h"
#include "koQueryTypes.h"
#include "koFilterManager.h"

#include <koStore.h>
#include <koBinaryStore.h>
#include <koTarStore.h>
#include <koStoreStream.h>
#include <kio/netaccess.h>

#include <komlWriter.h>
#include <komlMime.h>
#include <komlStreamFeed.h>

#include <klocale.h>
#include <kapp.h>
#include <kdebug.h>
#include <kmessagebox.h>

#include <qpainter.h>
#include <qcolor.h>
#include <qpicture.h>
#include <qdom.h>

// Define the protocol used here for embedded documents' URL
// This used to "store:" but KURL didn't like it,
// so let's simply make it "tar:" !
#define STORE_PROTOCOL "tar:"
#define STORE_PROTOCOL_LENGTH 4
// Warning, keep it sync in koTarStore.cc

using namespace std;

/**********************************************************
 *
 * KoDocument
 *
 **********************************************************/

class KoDocumentPrivate
{
public:
  KoDocumentPrivate()
  {
    m_children.setAutoDelete( true );
  }
  ~KoDocumentPrivate()
  {
  }

  QList<KoView> m_views;
  QList<KoDocumentChild> m_children;
  QList<KoMainWindow> m_shells;

  bool m_bSingleViewMode;
};

KoDocument::KoDocument( QObject* parent, const char* name, bool singleViewMode )
    : KParts::ReadWritePart( parent, name )
{
    d = new KoDocumentPrivate;
    m_bEmpty = TRUE;

    d->m_bSingleViewMode = singleViewMode;

    // the parent setting *always* overrides! (Simon)
    if ( parent )
    {
      if ( parent->inherits( "KoDocument" ) )
        d->m_bSingleViewMode = ((KoDocument *)parent)->singleViewMode();
      else if ( parent->inherits( "KParts::Part" ) )
        d->m_bSingleViewMode = true;
    }
}

KoDocument::~KoDocument()
{
  d->m_shells.setAutoDelete( true );
  d->m_shells.clear();

  delete d;
}

bool KoDocument::singleViewMode() const
{
  return d->m_bSingleViewMode;
}

bool KoDocument::saveFile()
{
  if ( !kapp->inherits( "KoApplication" ) )
    return false;

  KMimeType::Ptr t = KMimeType::findByURL( m_url, 0, TRUE );
  QCString outputMimeType = t->mimeType().latin1();

  QApplication::setOverrideCursor( waitCursor );

  if ( KIO::NetAccess::exists( m_url ) ) { // this file exists => backup
	// TODO : make this configurable ?
        KURL backup( m_url );
        backup.setPath( backup.path() + QString::fromLatin1("~") );
        (void) KIO::NetAccess::del( backup );
        (void) KIO::NetAccess::copy( m_url, backup );

        //QString cmd = QString( "rm -rf %1~" ).arg( url.path() );
        //system( cmd.local8Bit() );
        //cmd = QString("cp %1 %2~").arg( url.path() ).arg( url.path() );
        //system( cmd.local8Bit() );
    }
  QCString _native_format = nativeFormatMimeType();
  bool ret;
  if ( outputMimeType != _native_format ) {
    // Not native format : save using export filter
    QString nativeFile=KoFilterManager::self()->prepareExport( m_file, _native_format);
    ret = saveNativeFormat( nativeFile ) && KoFilterManager::self()->export_();
  } else {
    // Native format => normal save
    ret = saveNativeFormat( m_file );
  }

  if ( !ret )
  {
    KMessageBox::error( 0L, i18n( "Could not save\n%1" ).arg( m_file ) );
  }
  QApplication::restoreOverrideCursor();
  return ret;
}

QWidget *KoDocument::widget()
{
  if ( !d->m_bSingleViewMode )
    return 0L;

  if ( d->m_views.count() == 0 )
  {
    QWidget *parentWidget = 0L;

    if ( parent() )
    {
      if ( parent()->inherits( "QWidget" ) )
        parentWidget = (QWidget *)parent();
      else if ( parent()->inherits( "KoDocument" ) )
      {
        KoDocument *parentDoc = (KoDocument *)parent();
	if ( parentDoc->singleViewMode() )
	  parentWidget = parentDoc->widget();
      }
    }

    QWidget *w = createView( parentWidget );
    assert( w );
    setWidget( w );
  }

  return d->m_views.getFirst();
}

QAction *KoDocument::action( const QDomElement &element )
{
  return d->m_views.getFirst()->action( element );
}

QDomDocument KoDocument::document() const
{
  return d->m_views.getFirst()->document();
}

void KoDocument::setManager( KParts::PartManager *manager )
{
  KParts::ReadWritePart::setManager( manager );
  if ( d->m_bSingleViewMode && d->m_views.count() == 1 )
    d->m_views.getFirst()->setPartManager( manager );
}

void KoDocument::setReadWrite( bool readwrite )
{
  KParts::ReadWritePart::setReadWrite( readwrite );

  QListIterator<KoView> vIt( d->m_views );
  for (; vIt.current(); ++vIt )
    vIt.current()->updateReadWrite( readwrite );

  QListIterator<KoDocumentChild> dIt( d->m_children );
  for (; dIt.current(); ++dIt )
    if ( dIt.current()->document() )
      dIt.current()->document()->setReadWrite( readwrite );
}

void KoDocument::addView( KoView *view )
{
  if ( !view )
    return;

  d->m_views.append( view );

  connect( view, SIGNAL( destroyed() ),
	   this, SLOT( slotViewDestroyed() ) );

  view->updateReadWrite( isReadWrite() );
}

KoView *KoDocument::firstView()
{
  return d->m_views.first();
}

KoView *KoDocument::nextView()
{
  return d->m_views.next();
}

void KoDocument::slotViewDestroyed()
{
  d->m_views.removeRef( (KoView *)sender() );
}

void KoDocument::insertChild( KoDocumentChild *child )
{
  setModified( true );

  d->m_children.append( child );

  connect( child, SIGNAL( changed( KoDocumentChild * ) ),
	   this, SIGNAL( childChanged( KoDocumentChild * ) ) );
}

QList<KoDocumentChild> &KoDocument::children() const
{
  return d->m_children;
}

KParts::Part *KoDocument::hitTest( QWidget *widget, const QPoint &globalPos )
{
  QListIterator<KoView> it( d->m_views );
  for (; it.current(); ++it )
    if ( (QWidget *)it.current() == widget )
    {
      QPoint canvasPos( it.current()->canvas()->mapFromGlobal( globalPos ) );
      canvasPos.rx() -= it.current()->canvasXOffset();
      canvasPos.ry() -= it.current()->canvasYOffset();

      KParts::Part *part = it.current()->hitTest( canvasPos );
      if ( part )
        return part;
    }

  return 0L;
}

KoDocument *KoDocument::hitTest( const QPoint &pos, const QWMatrix &matrix )
{
  QListIterator<KoDocumentChild> it( d->m_children );
  for (; it.current(); ++it )
  {
    KoDocument *doc = it.current()->hitTest( pos, matrix );
    if ( doc )
      return doc;
  }

  return this;
}

KoDocumentChild *KoDocument::child( KoDocument *doc )
{
  QListIterator<KoDocumentChild> it( d->m_children );
  for (; it.current(); ++it )
    if ( it.current()->document() == doc )
      return it.current();

  return 0L;
}

void KoDocument::paintEverything( QPainter &painter, const QRect &rect, bool transparent, KoView *view )
{
  paintContent( painter, rect, transparent );
  paintChildren( painter, rect, view );
}

void KoDocument::paintChildren( QPainter &painter, const QRect &/*rect*/, KoView *view )
{
  QListIterator<KoDocumentChild> it( d->m_children );
  for (; it.current(); ++it )
  {
    // #### todo: paint only if child is visible inside rect
    painter.save();
    paintChild( it.current(), painter, view );
    painter.restore();
  }
}

void KoDocument::paintChild( KoDocumentChild *child, QPainter &painter, KoView *view )
{
  QRegion rgn = painter.clipRegion();

  child->transform( painter );
  child->document()->paintEverything( painter, child->contentRect(), child->isTransparent(), view );

  if ( view && view->partManager() )
  {
    KParts::PartManager *manager = view->partManager();

    painter.scale( 1.0 / child->xScaling(), 1.0 / child->yScaling() );

    int w = int( (double)child->contentRect().width() * child->xScaling() );
    int h = int( (double)child->contentRect().height() * child->yScaling() );
    if ( ( manager->selectedPart() == (KParts::Part *)child->document() &&
	   manager->selectedWidget() == (QWidget *)view ) ||
	 ( manager->activePart() == (KParts::Part *)child->document() &&
	   manager->activeWidget() == (QWidget *)view ) )
        {
	  painter.setClipRegion( rgn );

	  painter.setPen( black );
	  painter.fillRect( -5, -5, w + 10, 5, white );
	  painter.fillRect( -5, h, w + 10, 5, white );
	  painter.fillRect( -5, -5, 5, h + 10, white );
	  painter.fillRect( w, -5, 5, h + 10, white );
	  painter.fillRect( -5, -5, w + 10, 5, BDiagPattern );
	  painter.fillRect( -5, h, w + 10, 5, BDiagPattern );		
	  painter.fillRect( -5, -5, 5, h + 10, BDiagPattern );
	  painter.fillRect( w, -5, 5, h + 10, BDiagPattern );
	
	  if ( manager->selectedPart() == (KParts::Part *)child->document() &&
	       manager->selectedWidget() == (QWidget *)view )
	  {
	    QColor color;
	    if ( view->koDocument() == this )
	      color = black;
	    else
	      color = gray;
	    painter.fillRect( -5, -5, 5, 5, color );
	    painter.fillRect( -5, h, 5, 5, color );
	    painter.fillRect( w, h, 5, 5, color );
	    painter.fillRect( w, -5, 5, 5, color );
	    painter.fillRect( w / 2 - 3, -5, 5, 5, color );
	    painter.fillRect( w / 2 - 3, h, 5, 5, color );
	    painter.fillRect( -5, h / 2 - 3, 5, 5, color );
	    painter.fillRect( w, h / 2 - 3, 5, 5, color );
	  }
      }
  }
}

bool KoDocument::saveChildren( KoStore* /*_store*/, const char * /*_path*/ )
{
  // Lets assume that we do not have children
  kDebugWarning( 30003, "KoDocument::saveChildren( KoStore*, const char * )");
  kDebugWarning( 30003, "Not implemented ( not really an error )" );
  return true;
}

bool KoDocument::saveNativeFormat( const QString & file )
{
  if ( hasToWriteMultipart() )
  {
    kDebugInfo( 30003, "Saving to store" );

    //Use this to save to a binary store (deprecated)
    //KoStore * store = new KoBinaryStore ( url.path(), KOStore::Write );

    KoStore* store = new KoTarStore( file, KoStore::Write );

    // Save childen first since they might get a new url
    if ( store->bad() || !saveChildren( store, STORE_PROTOCOL ) )
    {
      delete store;
      return false;
    }

    kDebugInfo( 30003, "Saving root" );
    if ( store->open( "root" ) )
    {
      ostorestream out( store );
      if ( !save( out, 0L /* to remove */ ) )
      {
	store->close();
	return false;
      }
      out.flush();
      store->close();
    }
    else
      return false;

    bool ret = completeSaving( store );
    kdDebug(30003) << "Saving done" << endl;
    delete store;
    return ret;
  }
  else
  {
    ofstream out( file );
    if ( !out )
    {
      KMessageBox::error( 0L, i18n("Could not write to\n%1" ).arg( file ) );
      return false;
    }

    return save( out, 0L /* to remove */ );
  }
}

bool KoDocument::saveToStore( KoStore* _store, const QCString & _format, const QString & _path )
{
  kDebugInfo( 30003, "Saving document to store" );

  // Use the path as the internal url
  m_url = _path;

  // Save childen first since they might get a new url
  if ( !saveChildren( _store, _path ) )
    return false;

  QString u = url().url();
  if ( _store->open( u, _format ) )
  {
    ostorestream out( _store );
    if ( !save( out, _format ) )
      return false;
    out.flush();
    _store->close();
  }

  if ( !completeSaving( _store ) )
    return false;

  kDebugInfo( 30003, "Saved document to store" );

  return true;
}

bool KoDocument::openFile()
{
  kdDebug(30003) << QString("KoDocument::openFile for %1").arg( m_file ) << endl;

  QApplication::setOverrideCursor( waitCursor );

  // Launch a filter if we need one for this url ?
  QString importedFile = KoFilterManager::self()->import( m_file, nativeFormatMimeType( instance() ) );

  QApplication::restoreOverrideCursor();

  // The filter, if any, has been applied. It's all native format now.
  bool loadOk = !importedFile.isEmpty(); // Empty = an error occured in the filter

  if (loadOk)
  {
    if ( !loadNativeFormat( importedFile ) )
    {
      loadOk = false;
      KMessageBox::error( 0L, i18n( "Could not open\n%1" ).arg(importedFile) );
    }
  }

  if ( importedFile != m_file )
  {
    // We opened a temporary file (result of an import filter)
    // Set document URL to empty - we don't want to save in /tmp !
    m_url = KURL();
    // and remove temp file
    unlink( importedFile.ascii() );
  }
  return loadOk;
}

bool KoDocument::loadNativeFormat( const QString & file )
{
  QApplication::setOverrideCursor( waitCursor );

  kDebugInfo( 30003, QString("KoDocument::loadNativeFormat( %1 )").arg( file ) );

  ifstream in( file );
  if ( !in )
  {
    QApplication::restoreOverrideCursor();
    return false;
  }

  // Try to find out whether it is a mime multi part file
  char buf[5];
  in.get( buf[0] ); in.get( buf[1] ); in.get( buf[2] ); in.get( buf[3] ); buf[4] = 0;
  in.unget(); in.unget(); in.unget(); in.unget();

  //kDebugInfo( 30003, "PATTERN=%s", buf );

  // Is it plain XML ?
  if ( strncasecmp( buf, "<?xm", 4 ) == 0 )
  {
    bool res = load( in, 0L );
    in.close();
    if ( res )
      res = completeLoading( 0L );

    QApplication::restoreOverrideCursor();
    return res;
  } else
  { // It's a koffice store (binary or tar.gz)
    in.close();
    KoStore * store;
    if ( strncasecmp( buf, "KS01", 4 ) == 0 )
    {
      store = new KoBinaryStore( file, KoStore::Read );
    }
    else // new (tar.gz)
    {
      store = new KoTarStore( file, KoStore::Read );
    }

    if ( store->bad() )
    {
      delete store;
      QApplication::restoreOverrideCursor();
      return false;
    }

    if ( store->open( "root", "" ) )
    {
      istorestream in( store );
      if ( !load( in, store ) )
      {
        delete store;
        QApplication::restoreOverrideCursor();
        return false;
      }
      store->close();
    }

    if ( !loadChildren( store ) )
    {	
      kDebugError( 30003, "ERROR: Could not load children" );
      delete store;
      QApplication::restoreOverrideCursor();
      return false;
    }

    bool res = completeLoading( store );
    delete store;
    QApplication::restoreOverrideCursor();
    return res;
  }
}

bool KoDocument::loadFromStore( KoStore* _store, const KURL & url )
{
  if ( _store->open( url.url(), "" ) )
  {
    istorestream in( _store );
    if ( !load( in, _store ) )
      return false;
    _store->close();
  }
  // Store as document URL
  m_url = url;

  if ( !loadChildren( _store ) )
  {	
    kDebugError( 30003, "ERROR: Could not load children" );
    return false;
  }

  return completeLoading( _store );
}

bool KoDocument::load( istream& in, KoStore* _store )
{
  kDebugInfo( 30003, "KoDocument::load( istream& in, KoStore* _store )");
  // Try to find out whether it is a mime multi part file
  char buf[5];
  in.get( buf[0] ); in.get( buf[1] ); in.get( buf[2] ); in.get( buf[3] ); buf[4] = 0;
  in.unget(); in.unget(); in.unget(); in.unget();

  kDebugInfo( 30003, "PATTERN2=%s", buf );

  // Load XML ?
  if ( strncasecmp( buf, "<?xm", 4 ) == 0 )
  {
    KOMLStreamFeed feed( in );
    KOMLParser parser( &feed );

    if ( !loadXML( parser, _store ) )
      return false;
  }
  // Load binary data
  else
  {
    if ( !loadBinary( in, false, _store ) )
      return false;
  }

  return true;
}

bool KoDocument::isStoredExtern()
{
  return ( m_url.protocol() != STORE_PROTOCOL );
}

void KoDocument::setModified( bool _mod )
{
    kdDebug() << "KoDocument::setModified( " << (_mod ? "true" : "false") << ")" << endl;
    KParts::ReadWritePart::setModified( _mod );

    if ( _mod )
	m_bEmpty = FALSE;
}

bool KoDocument::loadBinary( istream& , bool, KoStore* )
{
    kDebugError( 30003, "KoDocument::loadBinary not implemented" );
    return false;
}

bool KoDocument::loadXML( KOMLParser&, KoStore*  )
{
    kDebugError( 30003, "KoDocument::loadXML not implemented" );
    return false;
}

bool KoDocument::loadChildren( KoStore* )
{
    return true;
}

bool KoDocument::completeLoading( KoStore* )
{
    return true;
}

bool KoDocument::completeSaving( KoStore* )
{
    return true;
}

bool KoDocument::save( ostream&, const char* )
{
    kDebugError( 30003, "KoDocument::save not implemented" );
    return false;
}

QString KoDocument::copyright() const
{
    return "";
}

QString KoDocument::comment() const
{
    return "";
}

bool KoDocument::hasToWriteMultipart()
{
    return FALSE;
}

QCString KoDocument::nativeFormatMimeType( KInstance *instance )
{
  QString instname = instance ? instance->instanceName() : kapp->instanceName();

  KService::Ptr service = KService::service( instname );

  if ( !service )
    return QCString();

  KDesktopFile deFile( service->desktopEntryPath(), true /*readonly*/);

  QString nativeType = deFile.readEntry( "X-KDE-NativeMimeType" );
  return nativeType.latin1();
}

void KoDocument::addShell( KoMainWindow *shell )
{
  d->m_shells.append( shell );
}

void KoDocument::removeShell( KoMainWindow *shell )
{
  d->m_shells.removeRef( shell );
}

#include "koDocument.moc"
