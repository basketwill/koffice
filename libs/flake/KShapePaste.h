/* This file is part of the KDE project
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2009 Thomas Zander <zander@kde.org>

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

#ifndef KSHAPEPASTE_H
#define KSHAPEPASTE_H

#include <KOdfPasteBase.h>
#include "flake_export.h"

#include <QList>

class KCanvasBase;
class KShapeLayer;
class KShape;

/**
 * Class for pasting shapes to the document
 */
class FLAKE_EXPORT KShapePaste : public KOdfPasteBase
{
public:
    /**
     * Contructor
     *
     * @param canvas The canvas on which the paste is done
     * @param parentLayer The layer on which the shapes will be pasted
     */
    KShapePaste(KCanvasBase *canvas, KShapeLayer *parentLayer);
    virtual ~KShapePaste();

    QList<KShape*> pastedShapes() const;

protected:
    /// reimplemented
    virtual bool process(const KXmlElement & body, KOdfStoreReader &odfStore);

    class Private;
    Private * const d;
};

#endif /* KOSHAPEPASTE_H */
