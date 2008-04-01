/* This file is part of the KDE project
   Copyright (C) 2006-2007 Thorsten Zachmann <zachmann@kde.org>

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

#include "KoPAMasterPage.h"

#include <QBuffer>
#include <KoGenStyle.h>
#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include <KoOdfStylesReader.h>
#include <KoOdfLoadingContext.h>

#include "KoPASavingContext.h"
#include "KoPALoadingContext.h"

KoPAMasterPage::KoPAMasterPage()
: KoPAPageBase()
{
    m_pageLayout = KoPageLayout::standardLayout();
    setName( "Standard" );
}

KoPAMasterPage::~KoPAMasterPage()
{
}

KoShape * KoPAMasterPage::cloneShape() const
{
    // TODO implement cloning
    return 0;
}

void KoPAMasterPage::saveOdf( KoShapeSavingContext & context ) const
{
    KoPASavingContext &paContext = static_cast<KoPASavingContext&>( context );

    KoGenStyle pageLayoutStyle = pageLayout().saveOasis();
    pageLayoutStyle.setAutoStyleInStylesDotXml( true );
    pageLayoutStyle.addAttribute( "style:page-usage", "all" );
    QString pageLayoutName( paContext.mainStyles().lookup( pageLayoutStyle, "pm" ) );

    KoGenStyle pageMaster( KoGenStyle::StyleMaster );
    pageMaster.addAttribute( "style:page-layout-name", pageLayoutName );
    pageMaster.addAttribute( "style:display-name", name() );

    KoXmlWriter &savedWriter = paContext.xmlWriter();

    QBuffer buffer;
    buffer.open( QIODevice::WriteOnly );
    KoXmlWriter xmlWriter( &buffer );

    paContext.setXmlWriter( xmlWriter );

    saveOdfPageContent( paContext );

    paContext.setXmlWriter( savedWriter );

    QString contentElement = QString::fromUtf8( buffer.buffer(), buffer.buffer().size() );
    pageMaster.addChildElement( paContext.masterPageElementName(), contentElement );
    paContext.addMasterPage( this, paContext.mainStyles().lookup( pageMaster, "Default" ) );
}

void KoPAMasterPage::loadOdfPageTag( const KoXmlElement &element, KoPALoadingContext &loadingContext )
{
    KoPAPageBase::loadOdfPageTag( element, loadingContext );
    if ( element.hasAttributeNS( KoXmlNS::style, "display-name" ) ) {
        setName( element.attributeNS( KoXmlNS::style, "display-name" ) );
    }
    else {
        setName( element.attributeNS( KoXmlNS::style, "name" ) );
    }
    QString pageLayoutName = element.attributeNS( KoXmlNS::style, "page-layout-name" );
    const KoOdfStylesReader& styles = loadingContext.odfLoadingContext().stylesReader();
    const KoXmlElement* masterPageStyle = styles.findStyle( pageLayoutName );
    KoPageLayout pageLayout = KoPageLayout::standardLayout();

    if ( masterPageStyle ) {
        pageLayout.loadOasis( *masterPageStyle );
    }

    setPageLayout( pageLayout );
}
