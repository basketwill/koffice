/* This file is part of the KDE project
   Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>

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

#include "KFrameShape.h"

#include <KXmlReader.h>
#include <kdebug.h>

class KFrameShape::Private
{
public:
    Private(const QString &ns, const QString &tag)
            : ns(ns)
            , tag(tag) {}

    const QString ns;
    const QString tag;
};

KFrameShape::KFrameShape(const QString &ns, const QString &tag)
        : d(new Private(ns, tag))
{
}

KFrameShape::~KFrameShape()
{
    delete d;
}

bool KFrameShape::loadOdfFrame(const KXmlElement &element, KShapeLoadingContext &context)
{
    const KXmlElement &frameElement(KoXml::namedItemNS(element, d->ns, d->tag));
    if (frameElement.isNull()) {
        kError(30006) << "frame element" << d->tag << "not found";
        return false;
    }

    return loadOdfFrameElement(frameElement, context);
}
