#include <qpainter.h>
#include <qiconset.h>
#include <kaction.h>
#include <kstdaction.h>
#include <klocale.h>

#include "karbon_view.h"
#include "karbon_factory.h"
#include "karbon_part.h"

#include <kdebug.h>

KarbonView::KarbonView( KarbonPart* part, QWidget* parent, const char* name )
    : KoView( part, parent, name )
{
    setInstance( KarbonFactory::instance() );
    setXMLFile( QString::fromLatin1("karbon.rc") );

    initActions();
    
    m_canvas = new VCanvas( this, part );
}

KarbonView::~KarbonView()
{
/*	// doesnt the parent remove this automatically ?
    delete m_canvas;
    m_canvas = 0L;    
*/
}

void
KarbonView::updateReadWrite( bool /*rw*/ )
{
}

void
KarbonView::resizeEvent( QResizeEvent* /*event*/ ) {
kdDebug(31000) << "****view.resizeevent" << endl;    

}

void
KarbonView::cut()
{
    kdDebug(31000) << "KarbonView::cut(): CUT called" << endl;
}

void
KarbonView::initActions()
{
    KStdAction::cut(this, SLOT( cut() ), actionCollection(), "cut" );
}

#include "karbon_view.moc"
