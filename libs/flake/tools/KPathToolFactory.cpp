/* This file is part of the KDE project
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
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

#include "KPathToolFactory_p.h"
#include "KPathTool_p.h"
#include "KPathShape.h"

#include <klocale.h>

KPathToolFactory::KPathToolFactory(QObject *parent)
        : KToolFactoryBase(parent, "PathToolFactoryId")
{
    setToolTip(i18n("Path editing tool"));
    setToolType(dynamicToolType());
    setIcon("editpath");
    setPriority(2);
    setActivationShapeId(KoPathShapeId);
}

KPathToolFactory::~KPathToolFactory()
{
}

KToolBase * KPathToolFactory::createTool(KCanvasBase *canvas)
{
    return new KPathTool(canvas);
}

#include <KPathToolFactory_p.moc>
