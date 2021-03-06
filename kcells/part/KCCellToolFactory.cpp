/* This file is part of the KDE project
   Copyright 2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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
   Boston, MA 02110-1301, USA.
*/

// Local
#include "KCCellToolFactory.h"

#include <klocale.h>

#include "KCCellTool.h"

KCCellToolFactory::KCCellToolFactory(QObject* parent, const QString& id)
        : KToolFactoryBase(parent, id)
{
    setToolTip(i18n("Cell Tool"));
    setIcon("kcells");
    setToolType("KCells");
    setPriority(0);
    setActivationShapeId("flake/always");
}

KCCellToolFactory::~KCCellToolFactory()
{
}

KToolBase* KCCellToolFactory::createTool(KCanvasBase* canvas)
{
    return new KCCellTool(canvas);
}

void KCCellToolFactory::setPriority(int priority)
{
    KToolFactoryBase::setPriority(priority);
}

void KCCellToolFactory::setToolTip(const QString& toolTip)
{
    KToolFactoryBase::setToolTip(toolTip);
}

void KCCellToolFactory::setIcon(const QString& icon)
{
    KToolFactoryBase::setIcon(icon);
}

#include "KCCellToolFactory.moc"
