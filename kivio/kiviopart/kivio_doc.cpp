/*
 * Kivio - Visual Modelling and Flowcharting
 * Copyright (C) 2000 theKompany.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include <qdom.h>
#include <qtextstream.h>
#include <qbuffer.h>

#include "tkunits.h"
#include "kivio_doc.h"
#include "kivio_page.h"
#include "kivio_shell.h"
#include "kivio_map.h"
#include "kivio_view.h"
#include "kivio_factory.h"

#include "kivio_common.h"
#include "kivio_group_stencil.h"
#include "kivio_icon_view.h"
#include "kivio_layer.h"
#include "kivio_painter.h"
#include "kivio_screen_painter.h"
#include "kivio_stencil.h"
#include "kivio_stencil_spawner_set.h"
#include "kivio_viewmanager_panel.h"

#include "stencilbarbutton.h"

#include <unistd.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kurl.h>
#include <kapp.h>
#include <cassert>
#include <qdatetime.h>
#include <klocale.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qtextstream.h>

#include <kstddirs.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <koTemplateChooseDia.h>
#include <koFilterManager.h>
#include <koStoreDevice.h>

//using namespace std;

/*****************************************************************************
 *
 * KivioDoc
 *
 *****************************************************************************/

QList<KivioDoc>* KivioDoc::s_docs = 0;
int KivioDoc::s_docId = 0;

KivioDoc::KivioDoc( QWidget *parentWidget, const char* widgetName, QObject* parent, const char* name, bool singleViewMode )
: KoDocument( parentWidget, widgetName, parent, name, singleViewMode )
{
  if (!s_docs)
    s_docs = new QList<KivioDoc>;

  s_docs->append(this);

  m_pLstSpawnerSets = new QList<KivioStencilSpawnerSet>;
  m_pLstSpawnerSets->setAutoDelete(true);

  setInstance( KivioFactory::global(), false );

  if ( !name )
  {
    QString tmp( "Document%1" );
    tmp = tmp.arg( s_docId++ );
    setName( tmp.latin1() );
  }

  m_pClipboard = NULL;

  m_iPageId = 1;
  m_pMap = 0L;
  m_bLoading = false;
  m_pMap = new KivioMap( this, "Map" );

  // Load autoLoadStencils in internal StencilSpawnerSet
  m_pInternalSet = new KivioStencilSpawnerSet("Kivio_Internal");
  QStringList list = instance()->dirs()->findAllResources("data",instance()->instanceName()+"/autoloadStencils/*",true,false);
  QStringList::ConstIterator pIt = list.begin();
  QStringList::ConstIterator pEnd = list.end();
  for (; pIt != pEnd; ++pIt )
  {
    m_pInternalSet->loadFile(*pIt);
  }

  m_units = (int)UnitPoint;

  viewItemList = new ViewItemList(this);
}

QList<KivioDoc>& KivioDoc::documents()
{
  if ( s_docs == 0 )
    s_docs = new QList<KivioDoc>;
  return *s_docs;
}

bool KivioDoc::initDoc()
{
  QString f;
  KoTemplateChooseDia::ReturnType ret;

  ret = KoTemplateChooseDia::choose(  KivioFactory::global(), f,
                                      "application/x-kivio", "*.flw", "Kivio",
                                      KoTemplateChooseDia::NoTemplates );

  if ( ret == KoTemplateChooseDia::File ) {
    KURL url;
    url.setPath(f);
    return openURL( url );
  } else
    if ( ret == KoTemplateChooseDia::Empty ) {
      KivioPage *t = createPage(true);
      m_pMap->addPage( t );
      resetURL();
      return true;
  } else
    return false;
}

KoView* KivioDoc::createViewInstance( QWidget* parent, const char* name )
{
  if (!name)
    name = "View";

  return new KivioView( parent, name, this );
}

QDomDocument KivioDoc::saveXML()
{
  QDomDocument doc( "kiviodoc" );
  doc.appendChild( doc.createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"UTF-8\"" ) );
  QDomElement kivio = doc.createElement( "kiviosheet" );
  kivio.setAttribute( "editor", "Kivio" );
  kivio.setAttribute( "mime", "application/x-kivio" );

  kivio.setAttribute( "units", m_units );

  gridData.save(kivio,"grid");

  QDomElement viewItemsElement = doc.createElement("ViewItems");
  kivio.appendChild(viewItemsElement);
  viewItemList->save(viewItemsElement);

  doc.appendChild(kivio);

  // Save the list of stencils spawners we have loaded.
  // We save these as the following:
  //
  // <KivioStencilSpawnerSet name="Basic Flowcharting Shapes">
  //   <KivioSMLStencilSpawner file="blah"/>
  //   <KivioSMLStencilSpawner file="blah"/>
  // </KivioStencilSpawnerSet>
  // ....
  //
  // This is so we can load them back in, and check that we actually
  // have all these spawners on disk.
  KivioStencilSpawnerSet *pSet = m_pLstSpawnerSets->first();
  while( pSet )
  {
    kivio.appendChild( pSet->saveXML( doc ) );

    pSet = m_pLstSpawnerSets->next();
  }


  QDomElement e = m_pMap->save(doc);
  kivio.appendChild(e);

  // Write it out to a tmp file
     QFile f("filedump.xml");
    if ( f.open(IO_WriteOnly) ) {    // file opened successfully
        QTextStream t( &f );        // use a text stream
        t << doc.toString();
        f.close();
    }

  setModified(false);
  return doc;
}

bool KivioDoc::loadXML( QIODevice *, const QDomDocument& doc )
{
    kdDebug() << "-LOAD Loading KivioDoc" << endl;
  m_bLoading = true;

  if ( doc.doctype().name() != "kiviodoc" ) {
    kdDebug() << "-LOAD invalid doc type" << endl;
    m_bLoading = false;
    return false;
  }

  QDomElement kivio = doc.documentElement();
  if ( kivio.attribute( "mime" ) != "application/x-kivio" ) {
    kdDebug() << "-LOAD Invalid mime type" << endl;
    m_bLoading = false;
    return false;
  }

  QDomNode node = kivio.firstChild();
  while( !node.isNull() )
  {
    QString name = node.nodeName();
    if( name == "map" )
    {
        if( !m_pMap->loadXML( node.toElement() ) )
        {
            m_bLoading = false;
            return false;
        }
    }
    else if( name == "KivioStencilSpawnerSet" )
    {
        QString desc = XmlReadString( node.toElement(), "desc", "_!@#$" );
        if( desc == "_!@#$" )
        {
            kdDebug() << "-LOAD Bad KivioStencilSpawnerSet found" << endl;
        }
        else
        {
            loadStencilSpawnerSet( desc );
        }
    }
    else if( name == "ViewItems" )
    {
        viewItemList->load(node.toElement());
    }
    else
    {
        kdDebug() << "-LOAD Unknown node " << name << endl;
    }

    node = node.nextSibling();
  }

  // <map>
  //QDomElement mymap = kivio.namedItem( "map" ).toElement();
  //if ( !mymap.isNull() )
  //  if ( !m_pMap->loadXML(mymap) ) {
  //    m_bLoading = false;
  //    return false;
  //  }

  setUnits(kivio.attribute("units","0").toInt());

  gridData.load(kivio,"grid");

  return true;
}

bool KivioDoc::loadStencilSpawnerSet( const QString &desc )
{
    KStandardDirs *dirs = KGlobal::dirs();
    QStringList dirList = dirs->findDirs("data", "kivio/stencils");
    QString rootDir;

    kdDebug() << "-LOAD StencilSpawnerSet" << endl;
    // Iterate through all data directories
    for( QStringList::Iterator it = dirList.begin(); it != dirList.end(); ++it )
    {
        rootDir = (*it);

        // Within each data directory, iterate through all directories looking
        // for a filename (dir) that matches the parameter
        QDir d(rootDir);
        d.setFilter( QDir::Dirs );
        d.setSorting( QDir::Name );

        const QFileInfoList *list = d.entryInfoList();
        QFileInfoListIterator it( *list );
        QFileInfo *fi;

        // Loop through the outer directories (like BasicFlowcharting)
        while( (fi=it.current()) )
        {
            if( fi->fileName() != "." &&
                fi->fileName() != ".." )
            {
                QDir innerD(fi->absFilePath() );
                innerD.setFilter( QDir::Dirs );
                innerD.setSorting( QDir::Name );

                const QFileInfoList *innerList = innerD.entryInfoList();
                QFileInfoListIterator innerIT( *innerList );
                QFileInfo *innerFI;

                // Loop through the inner directories (like FlowChartingShapes1)
                while( (innerFI = innerIT.current()) )
                {
                    if( innerFI->fileName() != ".." &&
                        innerFI->fileName() != "." )
                    {
                        // Compare the descriptions
                        QString foundDesc;

                        foundDesc = KivioStencilSpawnerSet::readDesc(innerFI->absFilePath());
                        if( foundDesc == desc)
                        {

                            // Load the spawner set with  rootDir + "/" + fi.fileName()
                            KivioStencilSpawnerSet *pSet = addSpawnerSetDuringLoad( innerFI->absFilePath() );
                            if( pSet )
                            {
                                kdDebug() << "-LOAD Sucessful load of KivioStencilSpawnerSet, " << pSet->spawners()->count() << " stencils" << endl;
                            }
                            else
                            {
                                kdDebug() << "-LOAD **** FAILED TO LOAD STENCIL SPAWNER SET " << innerFI->absFilePath() << " ****" << endl;
                            }
                            return true;
                        }
                    }
                    ++innerIT;
                }
            }
            ++it;
        }
    }

    return false;
}

bool KivioDoc::completeLoading( KoStore* )
{
  m_bLoading = false;
  m_pMap->update();
  setModified( false );
  return true;
}

KivioPage* KivioDoc::createPage( bool useDefaults )
{
  QString s( i18n("Page%1") );
  s = s.arg( m_iPageId++ );

  KivioPage* t = new KivioPage(m_pMap,s.latin1()); // ### unicode fixme!! (Simon)
  t->setPageName(s,true);

  // Launch the page properties dialog.  If the user cancels,
  // remove it from the page map, and delete the page.  It has
  // not yet been added as a tab so we don't need to worry
  // about it.
  if( useDefaults == false )
  {
      if( t->pagePropertiesDlg() == false )
      {
        m_pMap->removePage(t);
        delete t;
        return NULL;
      }
  }

  return t;
}

void KivioDoc::addPage( KivioPage* page )
{
  m_pMap->addPage(page);
  setModified(true);
  emit sig_addPage(page);
}

void KivioDoc::paintContent( QPainter& /*painter*/, const QRect& /*rect*/, bool /*transparent*/ )
{
//  KivioPage* page = m_pMap->activePage();
//  if ( !page )
//    return;

//  paintContent(painter,rect,transparent,page);
}

void KivioDoc::paintContent( KivioPainter& painter, const QRect& rect, bool transparent, KivioPage* page, QPoint p0, float zoom )
{
  if ( isLoading() )
    return;

  page->paintContent(painter,rect,transparent,p0,zoom);
}

void KivioDoc::printContent( QPrinter &prn )
{
    KivioScreenPainter p;
    int from = prn.fromPage();
    int to = prn.toPage();
    int i;

    KivioPage *pPage;

    kdDebug() << "Printing from " << from << " to " << to << endl;

    p.start(&prn);
    for( i=from; i<=to; i++ )
    {
        pPage = m_pMap->pageList().at(i-1);
        pPage->printContent(p);

        if( i<to )
            prn.newPage();
    }
    p.stop();


//    pPage =
}

bool KivioDoc::setIsAlreadyLoaded( QString dirName, QString name )
{
    KivioStencilSpawnerSet *pSet = m_pLstSpawnerSets->first();
    while(pSet)
    {
        if( pSet->dir() == dirName || pSet->name() == name )
        {
            return true;
        }

        pSet = m_pLstSpawnerSets->next();
    }

    return false;
}

KivioStencilSpawnerSet *KivioDoc::addSpawnerSet( QString dirName )
{
    KivioStencilSpawnerSet *set;

    QString desc = KivioStencilSpawnerSet::readDesc( dirName );

    if( setIsAlreadyLoaded( dirName, desc ) )
    {
        kdDebug() << "Cannot load duplicate stencil sets" << endl;
        return NULL;
    }

    set = new KivioStencilSpawnerSet();
    if( set->loadDir(dirName)==false )
    {
        kdDebug() << "Error loading dir set (KivioDoc)\n" << endl;
        delete set;
        return NULL;
    }

    m_pLstSpawnerSets->append( set );
    setModified(true);

    kdDebug() << "-LOAD addSpawnerSet()  emitting signal" << endl;
    emit sig_addSpawnerSet( set );
    kdDebug() << "-LOAD after emit" << endl;

    return set;
}

KivioStencilSpawnerSet *KivioDoc::addSpawnerSetDuringLoad( QString dirName )
{
    KivioStencilSpawnerSet *set;

    set = new KivioStencilSpawnerSet();
    if( set->loadDir(dirName)==false )
    {
        kdDebug() << "Error loading dir set (KivioDOc)\n" << endl;
        delete set;
        return NULL;
    }

    m_pLstSpawnerSets->append( set );

    return set;
}

KivioDoc::~KivioDoc()
{
    // ***MUST*** Delete the pages first because they may
    // contain plugins which will be unloaded soon.  The stencils which are
    // spawned by plugins NEED the plugins still loaded when their destructor
    // is called or the program will slit it's throat.
    delete m_pMap;

    if( m_pClipboard )
    {
        delete m_pClipboard;
        m_pClipboard = NULL;
    }

    kdDebug() << "About to delete m_pLstSpawnerSets" << endl;
    if( m_pLstSpawnerSets )
    {
        delete m_pLstSpawnerSets;
        m_pLstSpawnerSets = NULL;
    }
    kdDebug() << "After delete m_pLstSpawnerSets" << endl;

    s_docs->removeRef(this);
}

bool KivioDoc::removeSpawnerSet( KivioStencilSpawnerSet *pSet )
{
    return m_pLstSpawnerSets->removeRef( pSet );
}

/**
 * Iterates through all spawner objects in the stencil set checking if
 * they exist in any of the pages.
 */
void KivioDoc::slotDeleteStencilSet( DragBarButton *pBtn, QWidget *w, KivioStackBar *pBar )
{
    // Iterate through all spawners in the set checking if they exist in any of
    // the pages
    KivioIconView *pIconView = (KivioIconView *)w;
    KivioStencilSpawnerSet *pSet = pIconView->spawnerSet();

    // Start the iteration
    KivioStencilSpawner *pSpawner = pSet->spawners()->first();
    while( pSpawner )
    {
        // Check for a spawner.  If there is one, the set cannot be deleted
        if( checkStencilsForSpawner( pSpawner )==true )
        {
            KMessageBox::error(NULL, i18n("Cannot delete stencil set because there are stencils using its spawners"),
                i18n("Cannot Delete Stencil Set"));
            return;
        }

        // Now check the clipboard against this spawner
        if( m_pClipboard )
        {
            if( checkGroupForSpawner( m_pClipboard, pSpawner )==true )
            {
                if( KMessageBox::questionYesNo(NULL, i18n("The clipboard contains stencils that use these spawners,\nwould you like to delete what is on the clipboard?\n(Saying no will cause this set of spawners to not be removed)"),
                    i18n("Clear The Clipboard?"))==KMessageBox::Yes )
                {
                    delete m_pClipboard;
                    m_pClipboard = NULL;
                }
                else    // abort because the user aborted
                    return;
            }
        }


        pSpawner = pSet->spawners()->next();
    }



    // If we made it this far, it's ok to delete this stencil set, so do it
//    if( KMessageBox::questionYesNo(NULL, i18n("Are you sure you want to delete this stencil set?"),
//        i18n("Delete Stencil Set?"))==KMessageBox::Yes )
    {
        // Destroying the IconView does not destroy the spawner set, so we remove
        // it here
        removeSpawnerSet( pIconView->spawnerSet() );

        // And emit the signal to kill the set (page & button)
        emit sig_deleteStencilSet( pBtn, w, pBar );
    }
}

/**
 * Checks if any stencils in the document use this spawner
 */
bool KivioDoc::checkStencilsForSpawner( KivioStencilSpawner *pSpawner )
{
    KivioPage *pPage;
    KivioLayer *pLayer;
    KivioStencil *pStencil;

    // Iterate across all the pages
    pPage = m_pMap->firstPage();
    while( pPage )
    {
        pLayer = pPage->layers()->first();
        while( pLayer )
        {
            pStencil = pLayer->stencilList()->first();
            while( pStencil )
            {
                // If this is a group stencil, then we must check all child stencils
                if( pStencil->groupList() && pStencil->groupList()->count() > 0 )
                {
                    if( checkGroupForSpawner( pStencil, pSpawner )==true )
                        return true;
                }
                else if( pStencil->spawner() == pSpawner )
                    return true;

                pStencil = pLayer->stencilList()->next();
            }

            pLayer = pPage->layers()->next();
        }

        pPage = m_pMap->nextPage();
    }

    return false;
}

bool KivioDoc::checkGroupForSpawner( KivioStencil *pGroup, KivioStencilSpawner *pSpawner )
{
    KivioStencil *pStencil;

    pStencil = pGroup->groupList()->first();
    while( pStencil )
    {
        if( pStencil->groupList() && pStencil->groupList()->count() > 0 )
        {
            if( checkGroupForSpawner( pStencil, pSpawner )==true )
                return true;
        }
        else if( pStencil->spawner() == pSpawner )
        {
            return true;
        }

        pStencil = pGroup->groupList()->next();
    }

    return false;
}

void KivioDoc::setClipboard( KivioGroupStencil *p )
{
    if( m_pClipboard )
        delete m_pClipboard;

    m_pClipboard = p;
}

KivioGroupStencil *KivioDoc::clipboard()
{
    return m_pClipboard;
}

void KivioDoc::slotSelectionChanged()
{
    emit sig_selectionChanged();
}

KivioStencilSpawner* KivioDoc::findStencilSpawner( const QString& setName, const QString& title )
{
    KivioStencilSpawnerSet *pSpawnerSet = m_pLstSpawnerSets->first();
    while( pSpawnerSet )
    {
        if( pSpawnerSet->name() == setName && pSpawnerSet->find(title) )
        {
            return pSpawnerSet->find(title);
        }
        pSpawnerSet = m_pLstSpawnerSets->next();
    }

    if( m_pInternalSet->name() == setName && m_pInternalSet->find(title) )
    {
        return m_pInternalSet->find(title);
    }

    return NULL;
}

KivioStencilSpawner* KivioDoc::findInternalStencilSpawner( const QString& title )
{
    return m_pInternalSet->find(title);
}

void KivioDoc::setUnits(int unit)
{
  if (m_units == unit)
    return;

  m_units = unit;
  emit unitsChanged(unit);
}

void KivioDoc::updateView(KivioPage* page, bool modified)
{
  emit sig_updateView(page);

  if (modified)
    setModified(true);
}
#include "kivio_doc.moc"
