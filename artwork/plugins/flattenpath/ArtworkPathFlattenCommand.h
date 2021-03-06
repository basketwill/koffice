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

#ifndef ARTWORKPATHFLATTENCOMMAND_H
#define ARTWORKPATHFLATTENCOMMAND_H

#include <QtGui/QUndoCommand>

class KPathShape;

/// The undo / redo command for flatten a given path
class ArtworkPathFlattenCommand : public QUndoCommand
{
public:
    /**
    * Command to flatten a path.
    *
    * The flatten works by inserting a specified number of points into
    * each path segment to get the given amount of flatness.
    *
    * @param path the path to flatten
    * @param flatness the desired flatness
    * @param parent the parent command used for macro commands
     */
    ArtworkPathFlattenCommand(KPathShape * path, qreal flatness, QUndoCommand *parent = 0);
    virtual ~ArtworkPathFlattenCommand();

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();

private:
    class Private;
    Private * const d;
};

#endif // ARTWORKPATHFLATTENCOMMAND_H
