/* This file is part of the KDE project
 * Copyright (C) 2008-2009 Jan Hambrecht <jaham@gmx.net>
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

#ifndef KSNAPSTRATEGY_H
#define KSNAPSTRATEGY_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Flake API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//


#include "KSnapGuide.h"

#include <QtCore/QPointF>
#include <QtGui/QPainterPath>

class KPathPoint;
class KSnapProxy;
class KViewConverter;

class KSnapStrategy
{
public:
    KSnapStrategy(KSnapGuide::Strategy type);
    virtual ~KSnapStrategy() {};

    virtual bool snap(const QPointF &mousePosition, KSnapProxy * proxy, qreal maxSnapDistance) = 0;

    /// returns the strategies type
    KSnapGuide::Strategy type() const;

    static qreal squareDistance(const QPointF &p1, const QPointF &p2);
    static qreal scalarProduct(const QPointF &p1, const QPointF &p2);

    /// returns the snapped position form the last call to snapToPoints
    QPointF snappedPosition() const;

    /// returns the current snap strategy decoration
    virtual QPainterPath decoration(const KViewConverter &converter) const = 0;

protected:
    /// sets the current snapped position
    void setSnappedPosition(const QPointF &position);

private:
    KSnapGuide::Strategy m_snapType;
    QPointF m_snappedPosition;
};

/// snaps to x- or y-coordinates of path points
class OrthogonalSnapStrategy : public KSnapStrategy
{
public:
    OrthogonalSnapStrategy();
    virtual bool snap(const QPointF &mousePosition, KSnapProxy * proxy, qreal maxSnapDistance);
    virtual QPainterPath decoration(const KViewConverter &converter) const;
private:
    QLineF m_hLine;
    QLineF m_vLine;
};

/// snaps to path points
class NodeSnapStrategy : public KSnapStrategy
{
public:
    NodeSnapStrategy();
    virtual bool snap(const QPointF &mousePosition, KSnapProxy * proxy, qreal maxSnapDistance);
    virtual QPainterPath decoration(const KViewConverter &converter) const;
};

/// snaps extension lines of path shapes
class ExtensionSnapStrategy : public KSnapStrategy
{
public:
    ExtensionSnapStrategy();
    virtual bool snap(const QPointF &mousePosition, KSnapProxy * proxy, qreal maxSnapDistance);
    virtual QPainterPath decoration(const KViewConverter &converter) const;
private:
    qreal project(const QPointF &lineStart , const QPointF &lineEnd, const QPointF &point);
    QPointF extensionDirection(KPathPoint * point, const QTransform &matrix);
    bool snapToExtension(QPointF &position, KPathPoint * point, const QTransform &matrix);
    QList<QLineF> m_lines;
};

/// snaps to intersections of shapes
class IntersectionSnapStrategy : public KSnapStrategy
{
public:
    IntersectionSnapStrategy();
    virtual bool snap(const QPointF &mousePosition, KSnapProxy * proxy, qreal maxSnapDistance);
    virtual QPainterPath decoration(const KViewConverter &converter) const;
};

/// snaps to the canvas grid
class GridSnapStrategy : public KSnapStrategy
{
public:
    GridSnapStrategy();
    virtual bool snap(const QPointF &mousePosition, KSnapProxy * proxy, qreal maxSnapDistance);
    virtual QPainterPath decoration(const KViewConverter &converter) const;
};

/// snaps to shape bounding boxes
class BoundingBoxSnapStrategy : public KSnapStrategy
{
public:
    BoundingBoxSnapStrategy();
    virtual bool snap(const QPointF &mousePosition, KSnapProxy * proxy, qreal maxSnapDistance);
    virtual QPainterPath decoration(const KViewConverter &converter) const;
private:
    qreal squareDistanceToLine(const QPointF &lineA, const QPointF &lineB, const QPointF &point, QPointF &pointOnLine);
    QPointF m_boxPoints[5];
};

/// snaps to line guides
class LineGuideSnapStrategy : public KSnapStrategy
{
public:
    LineGuideSnapStrategy();
    virtual bool snap(const QPointF &mousePosition, KSnapProxy * proxy, qreal maxSnapDistance);
    virtual QPainterPath decoration(const KViewConverter &converter) const;
private:
    int m_orientation;
};

#endif // KOSNAPSTRATEGY_H
