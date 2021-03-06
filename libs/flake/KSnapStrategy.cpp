/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
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

#include "KSnapStrategy_p.h"
#include "KSnapProxy_p.h"
#include "KSnapGuide.h"
#include <KPathShape.h>
#include <KPathPoint.h>
#include <KCanvasBase.h>
#include <KViewConverter.h>
#include <KGuidesData.h>

#include <QtGui/QPainter>

//#include <kdebug.h>
#include <math.h>


KSnapStrategy::KSnapStrategy(KSnapGuide::Strategy type)
        : m_snapType(type)
{
}

QPointF KSnapStrategy::snappedPosition() const
{
    return m_snappedPosition;
}

void KSnapStrategy::setSnappedPosition(const QPointF &position)
{
    m_snappedPosition = position;
}

KSnapGuide::Strategy KSnapStrategy::type() const
{
    return m_snapType;
}

qreal KSnapStrategy::squareDistance(const QPointF &p1, const QPointF &p2)
{
    qreal dx = p1.x() - p2.x();
    qreal dy = p1.y() - p2.y();
    return dx*dx + dy*dy;
}

qreal KSnapStrategy::scalarProduct(const QPointF &p1, const QPointF &p2)
{
    return p1.x() * p2.x() + p1.y() * p2.y();
}

OrthogonalSnapStrategy::OrthogonalSnapStrategy()
        : KSnapStrategy(KSnapGuide::OrthogonalSnapping)
{
}

bool OrthogonalSnapStrategy::snap(const QPointF &mousePosition, KSnapProxy * proxy, qreal maxSnapDistance)
{
    QPointF horzSnap, vertSnap;
    qreal minVertDist = HUGE_VAL;
    qreal minHorzDist = HUGE_VAL;

    QList<KShape*> shapes = proxy->shapes();
    foreach(KShape * shape, shapes) {
        QList<QPointF> points = proxy->pointsFromShape(shape);
        foreach (const QPointF &point, points) {
            qreal dx = fabs(point.x() - mousePosition.x());
            if (dx < minHorzDist && dx < maxSnapDistance) {
                minHorzDist = dx;
                horzSnap = point;
            }
            qreal dy = fabs(point.y() - mousePosition.y());
            if (dy < minVertDist && dy < maxSnapDistance) {
                minVertDist = dy;
                vertSnap = point;
            }
        }
    }

    QPointF snappedPoint = mousePosition;

    if (minHorzDist < HUGE_VAL)
        snappedPoint.setX(horzSnap.x());
    if (minVertDist < HUGE_VAL)
        snappedPoint.setY(vertSnap.y());

    if (minHorzDist < HUGE_VAL)
        m_hLine = QLineF(horzSnap, snappedPoint);
    else
        m_hLine = QLineF();

    if (minVertDist < HUGE_VAL)
        m_vLine = QLineF(vertSnap, snappedPoint);
    else
        m_vLine = QLineF();

    setSnappedPosition(snappedPoint);

    return (minHorzDist < HUGE_VAL || minVertDist < HUGE_VAL);
}

QPainterPath OrthogonalSnapStrategy::decoration(const KViewConverter &converter) const
{
    Q_UNUSED(converter);

    QPainterPath decoration;
    if (! m_hLine.isNull()) {
        decoration.moveTo(m_hLine.p1());
        decoration.lineTo(m_hLine.p2());
    }
    if (! m_vLine.isNull()) {
        decoration.moveTo(m_vLine.p1());
        decoration.lineTo(m_vLine.p2());
    }
    return decoration;
}

NodeSnapStrategy::NodeSnapStrategy()
        : KSnapStrategy(KSnapGuide::NodeSnapping)
{
}

bool NodeSnapStrategy::snap(const QPointF &mousePosition, KSnapProxy * proxy, qreal maxSnapDistance)
{
    qreal maxDistance = maxSnapDistance * maxSnapDistance;
    qreal minDistance = HUGE_VAL;

    QRectF rect(-maxSnapDistance, -maxSnapDistance, maxSnapDistance, maxSnapDistance);
    rect.moveCenter(mousePosition);
    QList<QPointF> points = proxy->pointsInRect(rect);

    QPointF snappedPoint = mousePosition;

    foreach (const QPointF &point, points) {
        qreal distance = squareDistance(mousePosition, point);
        if (distance < maxDistance && distance < minDistance) {
            snappedPoint = point;
            minDistance = distance;
        }
    }

    setSnappedPosition(snappedPoint);

    return (minDistance < HUGE_VAL);
}

QPainterPath NodeSnapStrategy::decoration(const KViewConverter &converter) const
{
    QRectF unzoomedRect = converter.viewToDocument(QRectF(0, 0, 11, 11));
    unzoomedRect.moveCenter(snappedPosition());
    QPainterPath decoration;
    decoration.addEllipse(unzoomedRect);
    return decoration;
}

ExtensionSnapStrategy::ExtensionSnapStrategy()
        : KSnapStrategy(KSnapGuide::ExtensionSnapping)
{
}

bool ExtensionSnapStrategy::snap(const QPointF &mousePosition, KSnapProxy * proxy, qreal maxSnapDistance)
{
    qreal maxDistance = maxSnapDistance * maxSnapDistance;
    qreal minDistances[2] = { HUGE_VAL, HUGE_VAL };

    QPointF snappedPoints[2] = { mousePosition, mousePosition };
    QPointF startPoints[2];

    QList<KShape*> shapes = proxy->shapes(true);
    foreach(KShape * shape, shapes) {
        KPathShape * path = dynamic_cast<KPathShape*>(shape);
        if (! path)
            continue;

        QTransform matrix = path->absoluteTransformation(0);

        int subpathCount = path->subpathCount();
        for (int subpathIndex = 0; subpathIndex < subpathCount; ++subpathIndex) {
            if (path->isClosedSubpath(subpathIndex))
                continue;

            int pointCount = path->subpathPointCount(subpathIndex);

            // check the extension from the start point
            KPathPoint * first = path->pointByIndex(KoPathPointIndex(subpathIndex, 0));
            QPointF firstSnapPosition = mousePosition;
            if (snapToExtension(firstSnapPosition, first, matrix)) {
                qreal distance = squareDistance(firstSnapPosition, mousePosition);
                if (distance < maxDistance) {
                    if (distance < minDistances[0]) {
                        minDistances[1] = minDistances[0];
                        snappedPoints[1] = snappedPoints[0];
                        startPoints[1] = startPoints[0];

                        minDistances[0] = distance;
                        snappedPoints[0] = firstSnapPosition;
                        startPoints[0] = matrix.map(first->point());
                    }
                    else if (distance < minDistances[1]) {
                        minDistances[1] = distance;
                        snappedPoints[1] = firstSnapPosition;
                        startPoints[1] = matrix.map(first->point());
                    }
                }
            }

            // now check the extension from the last point
            KPathPoint * last = path->pointByIndex(KoPathPointIndex(subpathIndex, pointCount - 1));
            QPointF lastSnapPosition = mousePosition;
            if (snapToExtension(lastSnapPosition, last, matrix)) {
                qreal distance = squareDistance(lastSnapPosition, mousePosition);
                if (distance < maxDistance) {
                    if (distance < minDistances[0]) {
                        minDistances[1] = minDistances[0];
                        snappedPoints[1] = snappedPoints[0];
                        startPoints[1] = startPoints[0];

                        minDistances[0] = distance;
                        snappedPoints[0] = lastSnapPosition;
                        startPoints[0] = matrix.map(last->point());
                    }
                    else if (distance < minDistances[1]) {
                        minDistances[1] = distance;
                        snappedPoints[1] = lastSnapPosition;
                        startPoints[1] = matrix.map(last->point());
                    }
                }
            }
        }
    }

    m_lines.clear();
    // if we have to extension near our mouse position, they might have an intersection
    // near our mouse position which we want to use as the snapped position
    if (minDistances[0] < HUGE_VAL && minDistances[1] < HUGE_VAL) {
        // check if intersection of extension lines is near mouse position
        KPathSegment s1(startPoints[0], snappedPoints[0] + snappedPoints[0]-startPoints[0]);
        KPathSegment s2(startPoints[1], snappedPoints[1] + snappedPoints[1]-startPoints[1]);
        QList<QPointF> isects = s1.intersections(s2);
        if (isects.count() == 1 && squareDistance(isects[0], mousePosition) < maxDistance) {
            // add both extension lines
            m_lines.append(QLineF(startPoints[0], isects[0]));
            m_lines.append(QLineF(startPoints[1], isects[0]));
            setSnappedPosition(isects[0]);
        }
        else {
            // only add nearest extension line of both
            uint index = minDistances[0] < minDistances[1] ? 0 : 1;
            m_lines.append(QLineF(startPoints[index], snappedPoints[index]));
            setSnappedPosition(snappedPoints[index]);
        }
    }
    else  if (minDistances[0] < HUGE_VAL) {
        m_lines.append(QLineF(startPoints[0], snappedPoints[0]));
        setSnappedPosition(snappedPoints[0]);
    }
    else if (minDistances[1] < HUGE_VAL) {
        m_lines.append(QLineF(startPoints[1], snappedPoints[1]));
        setSnappedPosition(snappedPoints[1]);
    }
    else {
        // none of the extension lines is near our mouse position
        return false;
    }

    return true;
}

QPainterPath ExtensionSnapStrategy::decoration(const KViewConverter &converter) const
{
    Q_UNUSED(converter);

    QPainterPath decoration;
    foreach (const QLineF &line, m_lines) {
        decoration.moveTo(line.p1());
        decoration.lineTo(line.p2());
    }
    return decoration;
}

bool ExtensionSnapStrategy::snapToExtension(QPointF &position, KPathPoint * point, const QTransform &matrix)
{
    QPointF direction = extensionDirection(point, matrix);
    if (direction.isNull())
        return false;

    QPointF extensionStart = matrix.map(point->point());
    QPointF extensionStop = matrix.map(point->point()) + direction;
    float posOnExtension = project(extensionStart, extensionStop, position);
    if (posOnExtension < 0.0)
        return false;

    position = extensionStart + posOnExtension * direction;
    return true;
}

qreal ExtensionSnapStrategy::project(const QPointF &lineStart, const QPointF &lineEnd, const QPointF &point)
{
    QPointF diff = lineEnd - lineStart;
    QPointF relPoint = point - lineStart;
    qreal diffLength = sqrt(diff.x() * diff.x() + diff.y() * diff.y());
    if (diffLength == 0.0)
        return 0.0;

    diff /= diffLength;
    // project mouse position relative to stop position on extension line
    qreal scalar = relPoint.x() * diff.x() + relPoint.y() * diff.y();
    return scalar /= diffLength;
}

QPointF ExtensionSnapStrategy::extensionDirection(KPathPoint * point, const QTransform &matrix)
{
    KPathShape * path = point->parent();
    KoPathPointIndex index = path->pathPointIndex(point);

    /// check if it is a start point
    if (point->properties() & KPathPoint::StartSubpath) {
        if (point->activeControlPoint2()) {
            return matrix.map(point->point()) - matrix.map(point->controlPoint2());
        } else {
            KPathPoint * next = path->pointByIndex(KoPathPointIndex(index.first, index.second + 1));
            if (! next)
                return QPointF();
            else if (next->activeControlPoint1())
                return matrix.map(point->point()) - matrix.map(next->controlPoint1());
            else
                return matrix.map(point->point()) - matrix.map(next->point());
        }
    } else {
        if (point->activeControlPoint1()) {
            return matrix.map(point->point()) - matrix.map(point->controlPoint1());
        } else {
            KPathPoint * prev = path->pointByIndex(KoPathPointIndex(index.first, index.second - 1));
            if (! prev)
                return QPointF();
            else if (prev->activeControlPoint2())
                return matrix.map(point->point()) - matrix.map(prev->controlPoint2());
            else
                return matrix.map(point->point()) - matrix.map(prev->point());
        }
    }
}

IntersectionSnapStrategy::IntersectionSnapStrategy()
        : KSnapStrategy(KSnapGuide::IntersectionSnapping)
{
}

bool IntersectionSnapStrategy::snap(const QPointF &mousePosition, KSnapProxy *proxy, qreal maxSnapDistance)
{
    qreal maxDistance = maxSnapDistance * maxSnapDistance;
    qreal minDistance = HUGE_VAL;

    QRectF rect(-maxSnapDistance, -maxSnapDistance, maxSnapDistance, maxSnapDistance);
    rect.moveCenter(mousePosition);
    QPointF snappedPoint = mousePosition;

    QList<KPathSegment> segments = proxy->segmentsInRect(rect);
    //kDebug() << "found" << segments.count() << "segments in roi";

    int segmentCount = segments.count();
    for (int i = 0; i < segmentCount; ++i) {
        const KPathSegment &s1 = segments[i];
        for (int j = i + 1; j < segmentCount; ++j) {
            QList<QPointF> isects = s1.intersections(segments[j]);
            //kDebug() << isects.count() << "intersections found";
            foreach(const QPointF &point, isects) {
                if (! rect.contains(point))
                    continue;
                qreal distance = squareDistance(mousePosition, point);
                if (distance < maxDistance && distance < minDistance) {
                    snappedPoint = point;
                    minDistance = distance;
                }
            }
        }
    }

    setSnappedPosition(snappedPoint);

    return (minDistance < HUGE_VAL);
}

QPainterPath IntersectionSnapStrategy::decoration(const KViewConverter &converter) const
{
    QRectF unzoomedRect = converter.viewToDocument(QRectF(0, 0, 11, 11));
    unzoomedRect.moveCenter(snappedPosition());
    QPainterPath decoration;
    decoration.addRect(unzoomedRect);
    return decoration;
}

GridSnapStrategy::GridSnapStrategy()
        : KSnapStrategy(KSnapGuide::GridSnapping)
{
}

bool GridSnapStrategy::snap(const QPointF &mousePosition, KSnapProxy *proxy, qreal maxSnapDistance)
{
    if (! proxy->canvas()->snapToGrid())
        return false;

    // The 1e-10 here is a workaround for some weird division problem.
    // 360.00062366 / 2.83465058 gives 127 'exactly' when shown as a qreal,
    // but when casting into an int, we get 126. In fact it's 127 - 5.64e-15 !
    qreal gridX, gridY;
    proxy->canvas()->gridSize(&gridX, &gridY);

    // we want to snap to the nearest grid point, so calculate
    // the grid rows/columns before and after the points position
    int col = static_cast<int>(mousePosition.x() / gridX + 1e-10);
    int nextCol = col + 1;
    int row = static_cast<int>(mousePosition.y() / gridY + 1e-10);
    int nextRow = row + 1;

    // now check which grid line has less distance to the point
    qreal distToCol = qAbs(col * gridX - mousePosition.x());
    qreal distToNextCol = qAbs(nextCol * gridX - mousePosition.x());
    if (distToCol > distToNextCol) {
        col = nextCol;
        distToCol = distToNextCol;
    }

    qreal distToRow = qAbs(row * gridY - mousePosition.y());
    qreal distToNextRow = qAbs(nextRow * gridY - mousePosition.y());
    if (distToRow > distToNextRow) {
        row = nextRow;
        distToRow = distToNextRow;
    }

    QPointF snappedPoint = mousePosition;

    qreal distance = distToCol * distToCol + distToRow * distToRow;
    qreal maxDistance = maxSnapDistance * maxSnapDistance;
    // now check if we are inside the snap distance
    if (distance < maxDistance) {
        snappedPoint = QPointF(col * gridX, row * gridY);
    }

    setSnappedPosition(snappedPoint);

    return (distance < maxDistance);
}

QPainterPath GridSnapStrategy::decoration(const KViewConverter &converter) const
{
    QSizeF unzoomedSize = converter.viewToDocument(QSizeF(5, 5));
    QPainterPath decoration;
    decoration.moveTo(snappedPosition() - QPointF(unzoomedSize.width(), 0));
    decoration.lineTo(snappedPosition() + QPointF(unzoomedSize.width(), 0));
    decoration.moveTo(snappedPosition() - QPointF(0, unzoomedSize.height()));
    decoration.lineTo(snappedPosition() + QPointF(0, unzoomedSize.height()));
    return decoration;
}

BoundingBoxSnapStrategy::BoundingBoxSnapStrategy()
        : KSnapStrategy(KSnapGuide::BoundingBoxSnapping)
{
}

bool BoundingBoxSnapStrategy::snap(const QPointF &mousePosition, KSnapProxy *proxy, qreal maxSnapDistance)
{
    qreal maxDistance = maxSnapDistance * maxSnapDistance;
    qreal minDistance = HUGE_VAL;

    QRectF rect(-maxSnapDistance, -maxSnapDistance, maxSnapDistance, maxSnapDistance);
    rect.moveCenter(mousePosition);
    QPointF snappedPoint = mousePosition;

    KFlake::Position pointId[5] = {
        KFlake::TopLeftCorner,
        KFlake::TopRightCorner,
        KFlake::BottomRightCorner,
        KFlake::BottomLeftCorner,
        KFlake::CenteredPosition
    };

    QList<KShape*> shapes = proxy->shapesInRect(rect, true);
    foreach(KShape * shape, shapes) {
        qreal shapeMinDistance = HUGE_VAL;
        // first check the corner and center points
        for (int i = 0; i < 5; ++i) {
            m_boxPoints[i] = shape->absolutePosition(pointId[i]);
            qreal d = squareDistance(mousePosition, m_boxPoints[i]);
            if (d < minDistance && d < maxDistance) {
                shapeMinDistance = d;
                minDistance = d;
                snappedPoint = m_boxPoints[i];
            }
        }
        // prioritize points over edges
        if (shapeMinDistance < maxDistance)
            continue;

        // now check distances to edges of bounding box
        for (int i = 0; i < 4; ++i) {
            QPointF pointOnLine;
            qreal d = squareDistanceToLine(m_boxPoints[i], m_boxPoints[(i+1)%4], mousePosition, pointOnLine);
            if (d < minDistance && d < maxDistance) {
                minDistance = d;
                snappedPoint = pointOnLine;
            }
        }
    }

    setSnappedPosition(snappedPoint);

    return (minDistance < maxDistance);

}

qreal BoundingBoxSnapStrategy::squareDistanceToLine(const QPointF &lineA, const QPointF &lineB, const QPointF &point, QPointF &pointOnLine)
{
    QPointF diff = lineB - lineA;
    qreal diffLength = sqrt(diff.x() * diff.x() + diff.y() * diff.y());
    if (diffLength == 0.0f)
        return HUGE_VAL;
    // project mouse position relative to start position on line
    qreal scalar = KSnapStrategy::scalarProduct(point - lineA, diff / diffLength);
    if (scalar < 0.0 || scalar > diffLength)
        return HUGE_VAL;
    // calculate vector between relative mouse position and projected mouse position
    pointOnLine = lineA + scalar / diffLength * diff;
    QPointF distVec = pointOnLine - point;
    return distVec.x()*distVec.x() + distVec.y()*distVec.y();
}

QPainterPath BoundingBoxSnapStrategy::decoration(const KViewConverter &converter) const
{
    QSizeF unzoomedSize = converter.viewToDocument(QSizeF(5, 5));

    QPainterPath decoration;
    decoration.moveTo(snappedPosition() - QPointF(unzoomedSize.width(), unzoomedSize.height()));
    decoration.lineTo(snappedPosition() + QPointF(unzoomedSize.width(), unzoomedSize.height()));
    decoration.moveTo(snappedPosition() - QPointF(unzoomedSize.width(), -unzoomedSize.height()));
    decoration.lineTo(snappedPosition() + QPointF(unzoomedSize.width(), -unzoomedSize.height()));

    return decoration;
}

LineGuideSnapStrategy::LineGuideSnapStrategy()
        : KSnapStrategy(KSnapGuide::GuideLineSnapping)
{
}

bool LineGuideSnapStrategy::snap(const QPointF &mousePosition, KSnapProxy * proxy, qreal maxSnapDistance)
{
    KGuidesData * guidesData = proxy->canvas()->guidesData();
    if (! guidesData || ! guidesData->showGuideLines())
        return false;

    QPointF snappedPoint = mousePosition;
    m_orientation = 0;

    qreal minHorzDistance = maxSnapDistance;
    foreach(qreal guidePos, guidesData->horizontalGuideLines()) {
        qreal distance = qAbs(guidePos - mousePosition.y());
        if (distance < minHorzDistance) {
            snappedPoint.setY(guidePos);
            minHorzDistance = distance;
            m_orientation |= Qt::Horizontal;
        }
    }
    qreal minVertSnapDistance = maxSnapDistance;
    foreach(qreal guidePos, guidesData->verticalGuideLines()) {
        qreal distance = qAbs(guidePos - mousePosition.x());
        if (distance < minVertSnapDistance) {
            snappedPoint.setX(guidePos);
            minVertSnapDistance = distance;
            m_orientation |= Qt::Vertical;
        }
    }

    setSnappedPosition(snappedPoint);

    return (minHorzDistance < maxSnapDistance || minVertSnapDistance < maxSnapDistance);
}

QPainterPath LineGuideSnapStrategy::decoration(const KViewConverter &converter) const
{
    QSizeF unzoomedSize = converter.viewToDocument(QSizeF(5, 5));
    QPainterPath decoration;
    if (m_orientation & Qt::Horizontal) {
        decoration.moveTo(snappedPosition() - QPointF(unzoomedSize.width(), 0));
        decoration.lineTo(snappedPosition() + QPointF(unzoomedSize.width(), 0));
    }
    if (m_orientation & Qt::Vertical) {
        decoration.moveTo(snappedPosition() - QPointF(0, unzoomedSize.height()));
        decoration.lineTo(snappedPosition() + QPointF(0, unzoomedSize.height()));
    }
    return decoration;
}
