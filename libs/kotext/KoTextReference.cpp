/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#include "KoTextReference_p.h"
#include "KoTextLocator.h"
#include "KoInlineTextObjectManager.h"

#include <KXmlReader.h> // for usage in Q_UNUSED
#include <KoShapeLoadingContext.h> // for usage in Q_UNUSED
#include <KoShapeSavingContext.h> // for usage in Q_UNUSED

KoTextReference::KoTextReference(int indexId)
        : KoVariable(),
        m_indexId(indexId)
{
}

KoTextReference::~KoTextReference()
{
    KoTextLocator *loc = locator();
    if (loc)
        loc->removeListener(this);
}

void KoTextReference::positionChanged()
{
    Q_ASSERT(manager());
    KoTextLocator *loc = locator();
    if (loc)
        setValue(QString::number(loc->pageNumber()));
    else
        setValue("NOREF"); // anything smarter to point to a broken reference?
}

void KoTextReference::setup()
{
    locator()->addListener(this);
    positionChanged();
}

KoTextLocator* KoTextReference::locator()
{
    return dynamic_cast<KoTextLocator*>(manager()->inlineTextObject(m_indexId));
}

bool KoTextReference::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    Q_UNUSED(element);
    Q_UNUSED(context);
    // TODO
    return false;
}

void KoTextReference::saveOdf(KoShapeSavingContext &context)
{
    Q_UNUSED(context);
    // TODO
}
