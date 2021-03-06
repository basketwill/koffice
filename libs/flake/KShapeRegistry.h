/* This file is part of the KDE project
 * Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
 * Copyright (C) 2006, 2010 Thomas Zander <zander@kde.org>
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

#ifndef KSHAPEREGISTRY_H
#define KSHAPEREGISTRY_H

#include <QObject>

#include <KGenericRegistry.h>
#include <KXmlReader.h>
#include <KShapeFactoryBase.h>

#include "flake_export.h"

class KShape;
class KShapeFactoryBase;
class KShapeLoadingContext;


/**
 * This singleton class keeps a register of all available flake shapes,
 * or rather, of the factories that applications can use to create flake
 * shape objects.
 */
class FLAKE_EXPORT KShapeRegistry : public QObject,  public KGenericRegistry<KShapeFactoryBase*>
{
    Q_OBJECT
public:
    ~KShapeRegistry();

    /**
     * Return an instance of the KShapeRegistry
     * Creates an instance if that has never happened before and returns the singleton instance.
     */
    static KShapeRegistry *instance();

    /**
     * Add shape factory for a shape that is not a plugin
     * This can be used also if you want to have a shape only in one application
     *
     * @param factory The factory of the shape
     */
    void addFactory(KShapeFactoryBase *factory);

    /**
     * Use the element to find out which flake plugin can load it, and
     * returns the loaded shape. The element expected is one of
     * 'draw:line', 'draw:frame' / etc.
     *
     * @returns the shape or 0 if no shape could be created. The shape may have as its parent
     *    set a layer which was previously created and stored in the context.
     * @see KShapeLoadingContext::layer()
     */
    KShape *createShapeFromOdf(const KXmlElement &element, KShapeLoadingContext &context) const;

private:
    KShapeRegistry();
    KShapeRegistry(const KShapeRegistry&);
    KShapeRegistry operator=(const KShapeRegistry&);

    class Private;
    Private *d;
};

#endif
