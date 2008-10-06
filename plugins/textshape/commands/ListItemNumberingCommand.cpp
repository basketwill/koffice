/* This file is part of the KDE project
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
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

#include "ListItemNumberingCommand.h"

#include <KoParagraphStyle.h>
#include <KoTextBlockData.h>
#include <QTextCursor>

ListItemNumberingCommand::ListItemNumberingCommand(const QTextBlock &block, bool numbered, QUndoCommand *parent)
    : TextCommandBase(parent),
      m_block(block),
      m_numbered(numbered)
{
    m_wasNumbered = !block.blockFormat().boolProperty(KoParagraphStyle::UnnumberedListItem);
}

ListItemNumberingCommand::~ListItemNumberingCommand()
{
}

void ListItemNumberingCommand::setNumbered(bool numbered)
{
    QTextCursor cursor(m_block);
    QTextBlockFormat blockFormat = cursor.blockFormat();
    if (numbered) {
        blockFormat.clearProperty(KoParagraphStyle::UnnumberedListItem);
    } else {
        blockFormat.setProperty(KoParagraphStyle::UnnumberedListItem, true);
    }
    cursor.setBlockFormat(blockFormat);

    KoTextBlockData *userData = dynamic_cast<KoTextBlockData*>(m_block.userData());
    if (userData)
        userData->setCounterWidth(-1.0);
}

void ListItemNumberingCommand::redo()
{
    TextCommandBase::redo();
    UndoRedoFinalizer finalizer(this, m_tool);

    setNumbered(m_numbered);
}

void ListItemNumberingCommand::undo()
{
    TextCommandBase::undo();
    UndoRedoFinalizer finalizer(this, m_tool);

    setNumbered(!m_numbered);
}

bool ListItemNumberingCommand::mergeWith(const QUndoCommand *other)
{
    const ListItemNumberingCommand *cmd = dynamic_cast<const ListItemNumberingCommand *>(other);
    if (!cmd)
        return false;
    if (m_block != cmd->m_block)
        return false;
    m_numbered = cmd->m_numbered;
    return true;
}

