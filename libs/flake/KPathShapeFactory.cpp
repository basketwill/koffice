/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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
#include "KPathShapeFactory_p.h"
#include "KPathShape.h"
#include "KLineBorder.h"
#include "KImageCollection.h"
#include "KResourceManager.h"
#include "KShapeLoadingContext.h"

#include <klocale.h>

#include <KXmlReader.h>
#include <KOdfXmlNS.h>

KPathShapeFactory::KPathShapeFactory(QObject *parent, const QStringList&)
        : KShapeFactoryBase(parent, KoPathShapeId, i18n("Simple path shape"))
{
    setToolTip(i18n("A simple path shape"));
    setIcon("pathshape");
    QStringList elementNames;
    elementNames << "path" << "line" << "polyline" << "polygon";
    setOdfElementNames(KOdfXmlNS::draw, elementNames);
    setLoadingPriority(0);
}

KShape *KPathShapeFactory::createDefaultShape(KResourceManager *) const
{
    KPathShape* path = new KPathShape();
    path->moveTo(QPointF(0, 50));
    path->curveTo(QPointF(0, 120), QPointF(50, 120), QPointF(50, 50));
    path->curveTo(QPointF(50, -20), QPointF(100, -20), QPointF(100, 50));
    path->normalize();
    path->setBorder(new KLineBorder(1.0));
    return path;
}

bool KPathShapeFactory::supports(const KXmlElement & e, KShapeLoadingContext &context) const
{
    Q_UNUSED(context);
    if (e.namespaceURI() == KOdfXmlNS::draw) {
        if (e.localName() == "path")
            return true;
        if (e.localName() == "line")
            return true;
        if (e.localName() == "polyline")
            return true;
        if (e.localName() == "polygon")
            return true;
    }

    return false;
}

void KPathShapeFactory::newDocumentResourceManager(KResourceManager *manager)
{
    // as we need an image collection for the pattern background
    // we want to make sure that there is always an image collection
    // added to the data center map, in case the picture shape plugin
    // is not loaded
    if (manager->imageCollection() == 0) {
        KImageCollection *imgCol = new KImageCollection(manager);
        manager->setImageCollection(imgCol);
    }
}
