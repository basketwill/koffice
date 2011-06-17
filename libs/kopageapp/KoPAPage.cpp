/* This file is part of the KDE project
   Copyright (C) 2006-2009 Thorsten Zachmann <zachmann@kde.org>

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

#include "KoPAPage.h"

#include <QPainter>
#include <KDebug>

#include <KShapePainter.h>
#include <KShapeSavingContext.h>
#include <KShapeLayer.h>
#include <KOdfLoadingContext.h>
#include <KOdfStyleStack.h>
#include <KXmlWriter.h>
#include <KOdfXmlNS.h>
#include <KoZoomHandler.h>

#include "KoPAMasterPage.h"
#include "KoPASavingContext.h"
#include "KoPALoadingContext.h"
#include "KoPAUtil.h"

KoPAPage::KoPAPage(KoPAMasterPage * masterPage)
: KoPAPageBase()
, m_masterPage(masterPage)
, m_pageProperties(UseMasterBackground | DisplayMasterBackground | DisplayMasterShapes)
{
    Q_ASSERT(masterPage);
}

KoPAPage::~KoPAPage()
{
}

void KoPAPage::saveOdf(KShapeSavingContext &context) const
{
    KoPASavingContext &paContext = static_cast<KoPASavingContext&>(context);

    paContext.xmlWriter().startElement("draw:page");
    paContext.xmlWriter().addAttribute("draw:name", paContext.pageName(this));
    if (!name().isEmpty() && name() != paContext.pageName(this)) {
        paContext.xmlWriter().addAttribute("koffice:name", name());
    }
    paContext.xmlWriter().addAttribute("draw:id", "page" + QString::number(paContext.page()));
    paContext.xmlWriter().addAttribute("draw:master-page-name", paContext.masterPageName(m_masterPage));
    paContext.xmlWriter().addAttribute("draw:style-name", saveOdfPageStyle(paContext));

    saveOdfPageContent(paContext);

    paContext.xmlWriter().endElement();
}

KOdfPageLayoutData &KoPAPage::pageLayout()
{
    Q_ASSERT(m_masterPage);

    return m_masterPage->pageLayout();
}

const KOdfPageLayoutData &KoPAPage::pageLayout() const
{
    Q_ASSERT(m_masterPage);

    return m_masterPage->pageLayout();
}

void KoPAPage::loadOdfPageTag(const KXmlElement &element, KoPALoadingContext &loadingContext)
{
    QString master = element.attributeNS (KOdfXmlNS::draw, "master-page-name");
    KoPAMasterPage *masterPage = loadingContext.masterPageByName(master);
    if (masterPage)
        setMasterPage(masterPage);
#ifndef NDEBUG
    else
        kWarning(30010) << "Loading didn't provide a page under name; " << master;
#endif
    KOdfStyleStack &styleStack = loadingContext.odfLoadingContext().styleStack();
    int pageProperties = UseMasterBackground | DisplayMasterShapes | DisplayMasterBackground;
    if (styleStack.hasProperty(KOdfXmlNS::draw, "fill")) {
        KoPAPageBase::loadOdfPageTag(element, loadingContext);
        pageProperties = DisplayMasterShapes;
    }
    m_pageProperties = pageProperties;
    QString name;
    if (element.hasAttributeNS(KOdfXmlNS::draw, "name")) {
        name = element.attributeNS(KOdfXmlNS::draw, "name");
        loadingContext.addPage(name, this);
    }
    if (element.hasAttributeNS(KOdfXmlNS::koffice, "name")) {
        name = element.attributeNS(KOdfXmlNS::koffice, "name");
    }
    setName(name);
}

void KoPAPage::setMasterPage(KoPAMasterPage * masterPage)
{
    Q_ASSERT(masterPage);
    m_masterPage = masterPage;
}

void KoPAPage::paintBackground(QPainter &painter, const KViewConverter &converter)
{
    if (m_pageProperties & UseMasterBackground) {
        if (m_pageProperties & DisplayMasterBackground) {
            Q_ASSERT(m_masterPage);
            m_masterPage->paintBackground(painter, converter);
        }
    }
    else {
        KoPAPageBase::paintBackground(painter, converter);
    }
}

bool KoPAPage::displayMasterShapes()
{
    return m_pageProperties & DisplayMasterShapes;
}

void KoPAPage::setDisplayMasterShapes(bool display)
{
    if (display) {
        m_pageProperties |= DisplayMasterShapes;
    }
    else {
        m_pageProperties &= ~DisplayMasterShapes;
    }
}

bool KoPAPage::displayMasterBackground()
{
    return m_pageProperties & UseMasterBackground;
}

void KoPAPage::setDisplayMasterBackground(bool display)
{
    if (display) {
        m_pageProperties |= UseMasterBackground;
    }
    else {
        m_pageProperties &= ~UseMasterBackground;
    }
}

bool KoPAPage::displayShape(KShape *shape) const
{
    Q_UNUSED(shape);
    return true;
}

QPixmap KoPAPage::generateThumbnail(const QSize &size)
{
    // don't paint null pixmap
    if (size.isEmpty()) // either width or height is <= 0
        return QPixmap();
    KoZoomHandler zoomHandler;
    const KOdfPageLayoutData & layout = pageLayout();
    KoPAUtil::setZoom(layout, size, zoomHandler);
    QRect pageRect(KoPAUtil::pageRect(layout, size, zoomHandler));

    QPixmap pixmap(size.width(), size.height());
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    painter.setClipRect(pageRect);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(pageRect.topLeft());

    paintPage(painter, zoomHandler);
    return pixmap;
}

void KoPAPage::paintPage(QPainter &painter, KoZoomHandler &zoomHandler)
{
    paintBackground(painter, zoomHandler);

    KShapePainter shapePainter(getPaintingStrategy());
    if (displayMasterShapes()) {
        shapePainter.setShapes(masterPage()->shapes());
        shapePainter.paint(painter, zoomHandler);
    }
    shapePainter.setShapes(shapes());
    shapePainter.paint(painter, zoomHandler);
}

void KoPAPage::saveOdfPageStyleData(KOdfGenericStyle &style, KoPASavingContext &paContext) const
{
    if ((m_pageProperties & UseMasterBackground) == 0) {
        KoPAPageBase::saveOdfPageStyleData(style, paContext);
    }
}
