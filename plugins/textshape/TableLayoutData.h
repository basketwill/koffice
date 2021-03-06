/* This file is part of the KDE project
 * Copyright (C) 2009 Elvis Stansvik <elvstone@gmail.org>
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

#ifndef TABLELAYOUTDATA_H
#define TABLELAYOUTDATA_H

#include <QtGlobal>
#include <QVector>
#include <QPointF>
#include <QRectF>

/**
 * @brief Table Rect struct.
 *
 * This struct holds layout helper data for table layout.
 *
 * \sa TableLayout
 */
struct TableRect
{
    int fromRow; /** First row in this table rect */
    QRectF rect; /** Rect occupied by table rect. */
    QVector<qreal> columnWidths;     /**< Column widths. */
    QVector<qreal> columnPositions;  /**< Column positions along X axis. Absolute positions */
};

/**
 * @brief Table layout data class.
 *
 * This class holds layout helper data for table layout.
 *
 * \sa TableLayout
 */
class TableLayoutData
{
public:
    /// Constructor.
    TableLayoutData();

private:
    friend class TestTableLayout; // To allow direct testing.
    friend class TableLayout;     // To allow direct manipulation during layout.

    QVector<qreal> m_rowHeights;       /**< Row heights. */
    QVector<qreal> m_rowPositions;     /**< Row positions along Y axis. Absolute positions */

    QVector<QVector<qreal> > m_contentHeights;  /**< Cell content heights. */
    QList<TableRect> m_tableRects; /**< Rects occupied by table, typically one per shape the table is in. */
    bool m_dirty;
    qreal m_minX;
    qreal m_maxX;
};

#endif // TABLELAYOUTDATA_H
