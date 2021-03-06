/* This file is part of the KDE project
   Copyright 2008 Stefan Nikolaus stefan.nikolaus@kdemail.net

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

#include "ReadWriteTableModel.h"

// KCells
#include "KCCell.h"
#include "KCSheet.h"

// Qt
#include <QSize>

class ReadWriteTableModel::Private
{
public:
};

ReadWriteTableModel::ReadWriteTableModel(KCSheet* sheet, int columns, int rows)
        : ReadOnlyTableModel(sheet, columns, rows)
        , d(new Private)
{
}

ReadWriteTableModel::~ReadWriteTableModel()
{
    delete d;
}

Qt::ItemFlags ReadWriteTableModel::flags(const QModelIndex& index) const
{
    if (!index.isValid() || !hasIndex(index.row(), index.column(), index.parent())) {
        return 0;
    }
    return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
}

bool ReadWriteTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    // NOTE Model indices start from 0, while KCells column/row indices start from 1.
    KCCell cell = KCCell(sheet(), index.column() + 1, index.row() + 1).masterCell();
    if (role == Qt::EditRole) {
        cell.parseUserInput(value.toString());
        emit dataChanged(index, index);
        return true;
    }
    return false;
}
