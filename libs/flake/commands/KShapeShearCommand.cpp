/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#include "KShapeShearCommand.h"
#include "KShape.h"

#include <klocale.h>

class KShapeShearCommandPrivate
{
public:
    QList<KShape*> shapes;
    QList<qreal> previousShearXs;
    QList<qreal> previousShearYs;
    QList<qreal> newShearXs;
    QList<qreal> newShearYs;
};

KShapeShearCommand::KShapeShearCommand(const QList<KShape*> &shapes, const QList<qreal> &previousShearXs, const QList<qreal> &previousShearYs, const QList<qreal> &newShearXs, const QList<qreal> &newShearYs, QUndoCommand *parent)
    : QUndoCommand(parent),
    d(new KShapeShearCommandPrivate())
{
    d->shapes = shapes;
    d->previousShearXs = previousShearXs;
    d->previousShearYs = previousShearYs;
    d->newShearXs = newShearXs;
    d->newShearYs = newShearYs;

    Q_ASSERT(d->shapes.count() == d->previousShearXs.count());
    Q_ASSERT(d->shapes.count() == d->previousShearYs.count());
    Q_ASSERT(d->shapes.count() == d->newShearXs.count());
    Q_ASSERT(d->shapes.count() == d->newShearYs.count());

    setText(i18n("Shear Shapes"));
}

KShapeShearCommand::~KShapeShearCommand()
{
    delete d;
}

void KShapeShearCommand::redo()
{
    QUndoCommand::redo();
    for (int i = 0; i < d->shapes.count(); i++) {
        d->shapes.at(i)->update();
        d->shapes.at(i)->shear(d->newShearXs.at(i), d->newShearYs.at(i));
        d->shapes.at(i)->update();
    }
}

void KShapeShearCommand::undo()
{
    QUndoCommand::undo();
    for (int i = 0; i < d->shapes.count(); i++) {
        d->shapes.at(i)->update();
        d->shapes.at(i)->shear(d->previousShearXs.at(i), d->previousShearYs.at(i));
        d->shapes.at(i)->update();
    }
}
