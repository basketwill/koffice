/* This file is part of the KDE project
 * Copyright (C) 2008 Peter Simonsson <peter.simonsson@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "CollectionShapeFactory.h"

#include <KShape.h>
#include <KDrag.h>
#include <KShapeOdfSaveHelper.h>
#include <KOdf.h>
#include <KShapeLoadingContext.h>
#include <KShapeControllerBase.h>
#include <KOdfLoadingContext.h>
#include <KOdfStore.h>
#include <KOdfStoreReader.h>
#include <KOdfXmlNS.h>
#include <KShapeRegistry.h>

#include <kdebug.h>

#include <QMimeData>
#include <QBuffer>

CollectionShapeFactory::CollectionShapeFactory(QObject *parent, const QString &id, KShape* shape)
    : KShapeFactoryBase(parent, id, shape->name()), m_shape(shape)
{
}

CollectionShapeFactory::~CollectionShapeFactory()
{
    delete m_shape;
}

KShape *CollectionShapeFactory::createDefaultShape(KResourceManager *documentResources) const
{
    QList<KShape*> shapes;

    shapes << m_shape;

    //kDebug() << m_shape->shapeId();

    KDrag drag;
    KShapeOdfSaveHelper saveHelper(shapes);
    drag.setOdf(KOdf::mimeType(KOdf::GraphicsDocument), saveHelper);
    QMimeData* data = drag.mimeData();

    QByteArray arr = data->data(KOdf::mimeType(KOdf::GraphicsDocument));
    KShape* shape = 0;

    if ( !arr.isEmpty() ) {
        QBuffer buffer( &arr );
        KOdfStore * store = KOdfStore::createStore( &buffer, KOdfStore::Read );
        KOdfStoreReader odfStore( store );

        QString errorMessage;
        if ( ! odfStore.loadAndParse( errorMessage ) ) {
            kError() << "loading and parsing failed:" << errorMessage << endl;
            return 0;
        }

        KXmlElement content = odfStore.contentDoc().documentElement();
        KXmlElement realBody( KoXml::namedItemNS( content, KOdfXmlNS::office, "body" ) );

        if ( realBody.isNull() ) {
            kError() << "No body tag found!" << endl;
            return 0;
        }

        KXmlElement body = KoXml::namedItemNS( realBody, KOdfXmlNS::office, KOdf::bodyContentElement( KOdf::TextDocument, false ) );

        if ( body.isNull() ) {
            kError() << "No" << KOdf::bodyContentElement(KOdf::TextDocument, true ) << "tag found!" << endl;
            return 0;
        }

        KOdfLoadingContext loadingContext(odfStore.styles(), odfStore.store());
        KShapeLoadingContext context(loadingContext, documentResources);

        KXmlElement element;

        forEachElement(element, body)
        {
            KShape * shape = KShapeRegistry::instance()->createShapeFromOdf( element, context );
            if ( shape ) {
                delete data;
                return shape;
            }
        }
    }

    delete data;
    return shape;
}

bool CollectionShapeFactory::supports(const KXmlElement &e, KShapeLoadingContext &context) const
{
    Q_UNUSED(e);
    Q_UNUSED(context);
    return false;
}
