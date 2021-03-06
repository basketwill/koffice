/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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

#include "KPathReverseCommand.h"
#include "KPathShape.h"
#include <klocale.h>

class KPathReverseCommand::Private
{
public:
    Private(const QList<KPathShape*> &p)
            : paths(p) {
    }
    ~Private() {
    }

    void reverse() {
        if (! paths.size())
            return;

        foreach(KPathShape* shape, paths) {
            int subpathCount = shape->subpathCount();
            for (int i = 0; i < subpathCount; ++i)
                shape->reverseSubpath(i);
        }
    }

    QList<KPathShape*> paths;
};

KPathReverseCommand::KPathReverseCommand(const QList<KPathShape*> &paths, QUndoCommand *parent)
        : QUndoCommand(parent),
        d(new Private(paths))
{
    setText(i18n("Reverse Paths"));
}

KPathReverseCommand::~KPathReverseCommand()
{
    delete d;
}

void KPathReverseCommand::redo()
{
    QUndoCommand::redo();

    d->reverse();
}

void KPathReverseCommand::undo()
{
    QUndoCommand::undo();

    d->reverse();
}
