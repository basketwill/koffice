 /* This file is part of the KDE project
   Copyright (C)  2002 Montel Laurent <lmontel@mandrakesoft.com>

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

#include <klocale.h>
#include "kpresenter_doc.h"
#include <qvbox.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include <kmessagebox.h>
#include "kprimportstyledia.h"
#include <koStore.h>
#include <qfile.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include <qlabel.h>

KPrImportStyleDia::KPrImportStyleDia( KPresenterDoc *_doc, const QStringList &_list, QWidget *parent, const char *name )
    :KoImportStyleDia( _list, parent, name ),
     m_doc(_doc)
{

}

KPrImportStyleDia::~KPrImportStyleDia()
{
}


void KPrImportStyleDia::loadFile()
{
    m_styleList.setAutoDelete(true);
    m_styleList.clear();
    m_listStyleName->clear();

    KFileDialog fd( QString::null, QString::null, 0, 0, TRUE );
    fd.setMimeFilter( "application/x-kpresenter" );
    fd.setCaption(i18n("Import Style"));
    KURL url;
    if ( fd.exec() != QDialog::Accepted )
        return;
    url = fd.selectedURL();
    if( url.isEmpty() )
    {
        KMessageBox::sorry( this,
                            i18n("File name is empty"),
                            i18n("Import Style"));
        return;
    }
    // ### TODO network transparency
    KoStore* store=KoStore::createStore( url.path(), KoStore::Read );
    if (store )
    {
        if (store->open("maindoc.xml") )
        {
            QDomDocument doc;
            doc.setContent( store->device() );
#if 0
            QDomElement word = doc.documentElement();

            QDomElement stylesElem = word.namedItem( "STYLES" ).toElement();
            if ( !stylesElem.isNull() )
            {
                //todo
                //duplicate code try to remove it !
                QValueList<QString> followingStyles;
                QDomNodeList listStyles = stylesElem.elementsByTagName( "STYLE" );
                for (unsigned int item = 0; item < listStyles.count(); item++)
                {
                    QDomElement styleElem = listStyles.item( item ).toElement();

                    KWStyle *sty = new KWStyle( QString::null );
                    // Load the paraglayout from the <STYLE> element
                    KoParagLayout lay = KoStyle::loadStyle( styleElem,m_doc->syntaxVersion() );
                    // This way, KWTextParag::setParagLayout also sets the style pointer, to this style
                    lay.style = sty;
                    sty->paragLayout() = lay;

                    QDomElement nameElem = styleElem.namedItem("NAME").toElement();
                    if ( !nameElem.isNull() )
                    {
                        sty->setName( nameElem.attribute("value") );
                        //kdDebug(32001) << "KWStyle created  name=" << sty->name() << endl;
                    } else
                        kdWarning() << "No NAME tag in LAYOUT -> no name for this style!" << endl;

                    // followingStyle is set by KWDocument::loadStyleTemplates after loading all the styles
                    sty->setFollowingStyle( sty );

                    QDomElement formatElem = styleElem.namedItem( "FORMAT" ).toElement();
                    if ( !formatElem.isNull() )
                        sty->format() = KWTextParag::loadFormat( formatElem, 0L, m_doc->defaultFont() );
                    else
                        kdWarning(32001) << "No FORMAT tag in <STYLE>" << endl; // This leads to problems in applyStyle().

                    // Style created, now let's try to add it
                    m_styleList.append(sty);

                    if(m_styleList.count() > followingStyles.count() )
                    {
                        QString following = styleElem.namedItem("FOLLOWING").toElement().attribute("name");
                        followingStyles.append( following );
                    }
                    else
                        kdWarning () << "Found duplicate style declaration, overwriting former " << sty->name() << endl;
                }

                Q_ASSERT( followingStyles.count() == m_styleList.count() );

                unsigned int i=0;
                for( QValueList<QString>::Iterator it = followingStyles.begin(); it != followingStyles.end(); ++it ) {
                    KWStyle * style = findStyle(*it);
                    if ( style )
                        m_styleList.at(i++)->setFollowingStyle( style );
                }

            }
#endif
            initList();
        }
        else
        {
            KMessageBox::error( this,
                                i18n("File is not a kword file!"),
                                i18n("Import Style"));
        }
        store->close();
    }
    delete store;
}

KoStyle *KPrImportStyleDia::findStyle( const QString & _name)
{
    QPtrListIterator<KoStyle> styleIt( m_styleList );
    for ( ; styleIt.current(); ++styleIt )
    {
        if ( styleIt.current()->name() == _name ) {
            return styleIt.current();
        }
    }
    return 0L;
}

void KPrImportStyleDia::generateStyleList()
{
}


#include "kprimportstyledia.moc"
