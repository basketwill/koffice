/* This file is part of the KDE project
 * Copyright (C) 2006,2008-2009 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006,2007 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2007, 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#include "KPathTool_p.h"
#include "KToolBase_p.h"
#include "KPathShape_p.h"
#include "KPathToolHandle_p.h"
#include "KCanvasBase.h"
#include "KShapeManager.h"
#include "KResourceManager.h"
#include "KViewConverter.h"
#include "KShapeSelection.h"
#include "KPointerEvent.h"
#include "commands/KPathPointTypeCommand_p.h"
#include "commands/KPathPointInsertCommand.h"
#include "commands/KPathPointRemoveCommand.h"
#include "commands/KPathSegmentTypeCommand.h"
#include "commands/KPathBreakAtPointCommand_p.h"
#include "commands/KPathSegmentBreakCommand_p.h"
#include "commands/KParameterToPathCommand.h"
#include "commands/KSubpathJoinCommand_p.h"
#include "commands/KPathPointMergeCommand.h"
#include "KParameterShape.h"
#include "KPathPoint.h"
#include "KPathPointRubberSelectStrategy_p.h"
#include "KPathSegmentChangeStrategy_p.h"
#include "PathToolOptionWidget_p.h"
#include "KSnapGuide.h"

#include <KAction>
#include <KIcon>
#include <KDebug>
#include <KLocale>
#include <QtGui/QPainter>
#include <QtGui/QBitmap>
#include <QtGui/QTabWidget>

static unsigned char needle_bits[] = {
    0x00, 0x00, 0x10, 0x00, 0x20, 0x00, 0x60, 0x00, 0xc0, 0x00, 0xc0, 0x01,
    0x80, 0x03, 0x80, 0x07, 0x00, 0x0f, 0x00, 0x1f, 0x00, 0x3e, 0x00, 0x7e,
    0x00, 0x7c, 0x00, 0x1c, 0x00, 0x18, 0x00, 0x00
};

static unsigned char needle_move_bits[] = {
    0x00, 0x00, 0x10, 0x00, 0x20, 0x00, 0x60, 0x00, 0xc0, 0x00, 0xc0, 0x01,
    0x80, 0x03, 0x80, 0x07, 0x10, 0x0f, 0x38, 0x1f, 0x54, 0x3e, 0xfe, 0x7e,
    0x54, 0x7c, 0x38, 0x1c, 0x10, 0x18, 0x00, 0x00
};

// helper function to calculate the squared distance between two points
qreal squaredDistance(const QPointF p1, const QPointF &p2)
{
    qreal dx = p1.x()-p2.x();
    qreal dy = p1.y()-p2.y();
    return dx*dx + dy*dy;
}

KPathTool::KPathTool(KCanvasBase *canvas)
        : KToolBase(canvas),
        m_activeHandle(0),
        m_handleRadius(3),
        m_pointSelection(this),
        m_currentStrategy(0),
        m_toolOptionWidget(0)
{
    setFlag(ToolHandleKeyEvents);
    QActionGroup *points = new QActionGroup(this);
    // m_pointTypeGroup->setExclusive(true);
    m_actionPathPointCorner = new KAction(KIcon("pathpoint-corner"), i18n("Corner point"), this);
    addAction("pathpoint-corner", m_actionPathPointCorner);
    m_actionPathPointCorner->setData(KPathPointTypeCommand::Corner);
    points->addAction(m_actionPathPointCorner);

    m_actionPathPointSmooth = new KAction(KIcon("pathpoint-smooth"), i18n("Smooth point"), this);
    addAction("pathpoint-smooth", m_actionPathPointSmooth);
    m_actionPathPointSmooth->setData(KPathPointTypeCommand::Smooth);
    points->addAction(m_actionPathPointSmooth);

    m_actionPathPointSymmetric = new KAction(KIcon("pathpoint-symmetric"), i18n("Symmetric Point"), this);
    addAction("pathpoint-symmetric", m_actionPathPointSymmetric);
    m_actionPathPointSymmetric->setData(KPathPointTypeCommand::Symmetric);
    points->addAction(m_actionPathPointSymmetric);

    m_actionCurvePoint = new KAction(KIcon("pathpoint-curve"), i18n("Make curve point"), this);
    addAction("pathpoint-curve", m_actionCurvePoint);
    connect(m_actionCurvePoint, SIGNAL(triggered()), this, SLOT(pointToCurve()));

    m_actionLinePoint = new KAction(KIcon("pathpoint-line"), i18n("Make line point"), this);
    addAction("pathpoint-line", m_actionLinePoint);
    connect(m_actionLinePoint, SIGNAL(triggered()), this, SLOT(pointToLine()));

    m_actionLineSegment = new KAction(KIcon("pathsegment-line"), i18n("Segment to Line"), this);
    m_actionLineSegment->setShortcut(Qt::Key_F);
    addAction("pathsegment-line", m_actionLineSegment);
    connect(m_actionLineSegment, SIGNAL(triggered()), this, SLOT(segmentToLine()));

    m_actionCurveSegment = new KAction(KIcon("pathsegment-curve"), i18n("Segment to Curve"), this);
    m_actionCurveSegment->setShortcut(Qt::Key_C);
    addAction("pathsegment-curve", m_actionCurveSegment);
    connect(m_actionCurveSegment, SIGNAL(triggered()), this, SLOT(segmentToCurve()));

    m_actionAddPoint = new KAction(KIcon("pathpoint-insert"), i18n("Insert point"), this);
    addAction("pathpoint-insert", m_actionAddPoint);
    m_actionAddPoint->setShortcut(Qt::Key_Insert);
    connect(m_actionAddPoint, SIGNAL(triggered()), this, SLOT(insertPoints()));

    m_actionRemovePoint = new KAction(KIcon("pathpoint-remove"), i18n("Remove point"), this);
    m_actionRemovePoint->setShortcut(Qt::Key_Backspace);
    addAction("pathpoint-remove", m_actionRemovePoint);
    connect(m_actionRemovePoint, SIGNAL(triggered()), this, SLOT(removePoints()));

    m_actionBreakPoint = new KAction(KIcon("path-break-point"), i18n("Break at point"), this);
    addAction("path-break-point", m_actionBreakPoint);
    connect(m_actionBreakPoint, SIGNAL(triggered()), this, SLOT(breakAtPoint()));

    m_actionBreakSegment = new KAction(KIcon("path-break-segment"), i18n("Break at segment"), this);
    addAction("path-break-segment", m_actionBreakSegment);
    connect(m_actionBreakSegment, SIGNAL(triggered()), this, SLOT(breakAtSegment()));

    m_actionJoinSegment = new KAction(KIcon("pathpoint-join"), i18n("Join with segment"), this);
    m_actionJoinSegment->setShortcut(Qt::Key_J);
    addAction("pathpoint-join", m_actionJoinSegment);
    connect(m_actionJoinSegment, SIGNAL(triggered()), this, SLOT(joinPoints()));

    m_actionMergePoints = new KAction(KIcon("pathpoint-merge"), i18n("Merge points"), this);
    addAction("pathpoint-merge", m_actionMergePoints);
    connect(m_actionMergePoints, SIGNAL(triggered()), this, SLOT(mergePoints()));

    m_actionConvertToPath = new KAction(KIcon("convert-to-path"), i18n("To Path"), this);
    m_actionConvertToPath->setShortcut(Qt::Key_P);
    addAction("convert-to-path", m_actionConvertToPath);
    connect(m_actionConvertToPath, SIGNAL(triggered()), this, SLOT(convertToPath()));

    connect(points, SIGNAL(triggered(QAction*)), this, SLOT(pointTypeChanged(QAction*)));
    connect(&m_pointSelection, SIGNAL(selectionChanged()), this, SLOT(pointSelectionChanged()));

    QBitmap b = QBitmap::fromData(QSize(16, 16), needle_bits);
    QBitmap m = b.createHeuristicMask(false);

    m_selectCursor = QCursor(b, m, 2, 0);

    b = QBitmap::fromData(QSize(16, 16), needle_move_bits);
    m = b.createHeuristicMask(false);

    m_moveCursor = QCursor(b, m, 2, 0);
}

KPathTool::~KPathTool()
{
}

QMap<QString, QWidget *>  KPathTool::createOptionWidgets()
{
    Q_D(KToolBase);
    QMap<QString, QWidget *> map;

    Q_ASSERT(m_toolOptionWidget == 0);
    m_toolOptionWidget = new PathToolOptionWidget(this);
    updateOptionsWidget();

    map.insert(i18n("Line/Curve"), m_toolOptionWidget);
    map.insert(i18n("Snapping"), d->canvas->createSnapGuideConfigWidget());

    return map;
}

void KPathTool::pointTypeChanged(QAction *type)
{
    Q_D(KToolBase);
    if (m_pointSelection.hasSelection()) {
        QList<KPathPointData> selectedPoints = m_pointSelection.selectedPointsData();
        QList<KPathPointData> pointToChange;

        QList<KPathPointData>::const_iterator it(selectedPoints.constBegin());
        for (; it != selectedPoints.constEnd(); ++it) {
            KPathPoint *point = it->pathShape->pointByIndex(it->pointIndex);
            if (point) {
                if (point->activeControlPoint1() && point->activeControlPoint2()) {
                    pointToChange.append(*it);
                }
            }
        }

        if (!pointToChange.isEmpty()) {
            KPathPointTypeCommand *cmd = new KPathPointTypeCommand(pointToChange,
                    static_cast<KPathPointTypeCommand::PointType>(type->data().toInt()));
            d->canvas->addCommand(cmd);
            updateActions();
        }
    }
}

void KPathTool::insertPoints()
{
    Q_D(KToolBase);
    if (m_pointSelection.size() > 1) {
        QList<KPathPointData> segments(m_pointSelection.selectedSegmentsData());
        if (!segments.isEmpty()) {
            KPathPointInsertCommand *cmd = new KPathPointInsertCommand(segments, 0.5);
            d->canvas->addCommand(cmd);

            foreach (KPathPoint * p, cmd->insertedPoints()) {
                m_pointSelection.add(p, false);
            }
            updateActions();
        }
    }
}

void KPathTool::removePoints()
{
    Q_D(KToolBase);
    // TODO finish current action or should this not possible during actions???
    if (m_pointSelection.size() > 0) {
        QUndoCommand *cmd = KPathPointRemoveCommand::createCommand(m_pointSelection.selectedPointsData(), d->canvas->shapeController());
        PointHandle *pointHandle = dynamic_cast<PointHandle*>(m_activeHandle);
        if (pointHandle && m_pointSelection.contains(pointHandle->activePoint())) {
            delete m_activeHandle;
            m_activeHandle = 0;
        }
        m_pointSelection.clear();
        d->canvas->addCommand(cmd);
    }
}

void KPathTool::pointToLine()
{
    Q_D(KToolBase);
    if (m_pointSelection.hasSelection()) {
        QList<KPathPointData> selectedPoints = m_pointSelection.selectedPointsData();
        QList<KPathPointData> pointToChange;

        QList<KPathPointData>::const_iterator it(selectedPoints.constBegin());
        for (; it != selectedPoints.constEnd(); ++it) {
            KPathPoint *point = it->pathShape->pointByIndex(it->pointIndex);
            if (point && (point->activeControlPoint1() || point->activeControlPoint2()))
                pointToChange.append(*it);
        }

        if (! pointToChange.isEmpty()) {
            d->canvas->addCommand(new KPathPointTypeCommand(pointToChange, KPathPointTypeCommand::Line));
            updateActions();
        }
    }
}

void KPathTool::pointToCurve()
{
    Q_D(KToolBase);
    if (m_pointSelection.hasSelection()) {
        QList<KPathPointData> selectedPoints = m_pointSelection.selectedPointsData();
        QList<KPathPointData> pointToChange;

        QList<KPathPointData>::const_iterator it(selectedPoints.constBegin());
        for (; it != selectedPoints.constEnd(); ++it) {
            KPathPoint *point = it->pathShape->pointByIndex(it->pointIndex);
            if (point && (! point->activeControlPoint1() || ! point->activeControlPoint2()))
                pointToChange.append(*it);
        }

        if (! pointToChange.isEmpty()) {
            d->canvas->addCommand(new KPathPointTypeCommand(pointToChange, KPathPointTypeCommand::Curve));
            updateActions();
        }
    }
}

void KPathTool::segmentToLine()
{
    Q_D(KToolBase);
    if (m_pointSelection.size() > 1) {
        QList<KPathPointData> segments(m_pointSelection.selectedSegmentsData());
        if (segments.size() > 0) {
            d->canvas->addCommand(new KPathSegmentTypeCommand(segments, KPathSegmentTypeCommand::Line));
            updateActions();
        }
    }
}

void KPathTool::segmentToCurve()
{
    Q_D(KToolBase);
    if (m_pointSelection.size() > 1) {
        QList<KPathPointData> segments(m_pointSelection.selectedSegmentsData());
        if (segments.size() > 0) {
            d->canvas->addCommand(new KPathSegmentTypeCommand(segments, KPathSegmentTypeCommand::Curve));
            updateActions();
        }
    }
}

void KPathTool::convertToPath()
{
    Q_D(KToolBase);
    QList<KParameterShape*> shapesToConvert;
    foreach(KShape *shape, m_pointSelection.selectedShapes()) {
        KParameterShape * parameterShape = dynamic_cast<KParameterShape*>(shape);
        if (parameterShape && parameterShape->isParametricShape())
            shapesToConvert.append(parameterShape);
    }
    if (shapesToConvert.count())
        d->canvas->addCommand(new KParameterToPathCommand(shapesToConvert));
    updateOptionsWidget();
}

void KPathTool::joinPoints()
{
    Q_D(KToolBase);
    if (m_pointSelection.objectCount() == 1 && m_pointSelection.size() == 2) {
        QList<KPathPointData> pd(m_pointSelection.selectedPointsData());
        const KPathPointData & pd1 = pd.at(0);
        const KPathPointData & pd2 = pd.at(1);
        KPathShape * pathShape = pd1.pathShape;
        if (!pathShape->isClosedSubpath(pd1.pointIndex.first) &&
                (pd1.pointIndex.second == 0 ||
                 pd1.pointIndex.second == pathShape->subpathPointCount(pd1.pointIndex.first) - 1) &&
                !pathShape->isClosedSubpath(pd2.pointIndex.first) &&
                (pd2.pointIndex.second == 0 ||
                 pd2.pointIndex.second == pathShape->subpathPointCount(pd2.pointIndex.first) - 1)) {
            KSubpathJoinCommand *cmd = new KSubpathJoinCommand(pd1, pd2);
            d->canvas->addCommand(cmd);
        }
        updateActions();
    }
}

void KPathTool::mergePoints()
{
    Q_D(KToolBase);
    if (m_pointSelection.objectCount() != 1 || m_pointSelection.size() != 2)
        return;

    QList<KPathPointData> pointData = m_pointSelection.selectedPointsData();
    const KPathPointData & pd1 = pointData.at(0);
    const KPathPointData & pd2 = pointData.at(1);
    const KoPathPointIndex & index1 = pd1.pointIndex;
    const KoPathPointIndex & index2 = pd2.pointIndex;

    KPathShape * path = pd1.pathShape;

    // check if subpaths are already closed
    if (path->isClosedSubpath(index1.first) || path->isClosedSubpath(index2.first))
        return;
    // check if first point is an endpoint
    if (index1.second != 0 && index1.second != path->subpathPointCount(index1.first)-1)
        return;
    // check if second point is an endpoint
    if (index2.second != 0 && index2.second != path->subpathPointCount(index2.first)-1)
        return;

    // now we can start merging the endpoints
    KPathPointMergeCommand *cmd = new KPathPointMergeCommand(pd1, pd2);
    d->canvas->addCommand(cmd);
    updateActions();
}

void KPathTool::breakAtPoint()
{
    Q_D(KToolBase);
    if (m_pointSelection.hasSelection()) {
        d->canvas->addCommand(new KPathBreakAtPointCommand(m_pointSelection.selectedPointsData()));
        updateActions();
    }
}

void KPathTool::breakAtSegment()
{
    Q_D(KToolBase);
    // only try to break a segment when 2 points of the same object are selected
    if (m_pointSelection.objectCount() == 1 && m_pointSelection.size() == 2) {
        QList<KPathPointData> segments(m_pointSelection.selectedSegmentsData());
        if (segments.size() == 1) {
            d->canvas->addCommand(new KPathSegmentBreakCommand(segments.at(0)));
            updateActions();
        }
    }
}

void KPathTool::paint(QPainter &painter, const KViewConverter &converter)
{
    Q_D(KToolBase);
    painter.setRenderHint(QPainter::Antialiasing, true);
    // use different colors so that it is also visible on a background of the same color
    painter.setBrush(Qt::white);   //TODO make configurable
    painter.setPen(Qt::blue);

    foreach(KPathShape *shape, m_pointSelection.selectedShapes()) {
        painter.save();
        painter.setTransform(shape->absoluteTransformation(&converter) * painter.transform());

        KParameterShape * parameterShape = dynamic_cast<KParameterShape*>(shape);
        if (parameterShape && parameterShape->isParametricShape()) {
            parameterShape->paintHandles(painter, converter, m_handleRadius);
        } else {
            shape->paintPoints(painter, converter, m_handleRadius);
        }

        painter.restore();
    }

    if (m_currentStrategy) {
        painter.save();
        m_currentStrategy->paint(painter, converter);
        painter.restore();
    }

    painter.setBrush(Qt::green);   // TODO make color configurable
    painter.setPen(Qt::blue);

    m_pointSelection.paint(painter, converter);

    painter.setBrush(Qt::red);   // TODO make color configurable
    painter.setPen(Qt::blue);

    if (m_activeHandle) {
        if (m_activeHandle->check(m_pointSelection.selectedShapes())) {
            m_activeHandle->paint(painter, converter);
        } else {
            delete m_activeHandle;
            m_activeHandle = 0;
        }
    }

    if (m_currentStrategy) {
        painter.save();
        KShape::applyConversion(painter, converter);
        d->canvas->snapGuide()->paint(painter, converter);
        painter.restore();
    }
}

void KPathTool::repaintDecorations()
{
    foreach(KShape *shape, m_pointSelection.selectedShapes()) {
        repaint(shape->boundingRect());
    }

    m_pointSelection.repaint();
    updateOptionsWidget();
}

void KPathTool::mousePressEvent(KPointerEvent *event)
{
    // we are moving if we hit a point and use the left mouse button
    event->ignore();
    if (m_activeHandle) {
        m_currentStrategy = m_activeHandle->handleMousePress(event);
        event->accept();
    } else {
        if (event->button() & Qt::LeftButton) {

            // check if we hit a path segment
            KPathShape * clickedShape = 0;
            KPathPoint * clickedPoint = 0;
            qreal clickedPointParam = 0.0;
            if (segmentAtPoint(event->point, clickedShape, clickedPoint, clickedPointParam)) {
                KoPathPointIndex index = clickedShape->pathPointIndex(clickedPoint);
                KPathPointData data(clickedShape, index);
                m_currentStrategy = new KPathSegmentChangeStrategy(this, event->point, data, clickedPointParam);
                event->accept();
            } else {
                if ((event->modifiers() & Qt::ControlModifier) == 0) {
                    m_pointSelection.clear();
                }
                // start rubberband selection
                Q_ASSERT(m_currentStrategy == 0);
                m_currentStrategy = new KPathPointRubberSelectStrategy(this, event->point);
            }
        }
    }
}

void KPathTool::mouseMoveEvent(KPointerEvent *event)
{
    if (event->button() & Qt::RightButton)
        return;

    if (m_currentStrategy) {
        m_lastPoint = event->point;
        m_currentStrategy->handleMouseMove(event->point, event->modifiers());

        // repaint new handle positions
        m_pointSelection.repaint();
        if (m_activeHandle)
            m_activeHandle->repaint();
        return;
    }

    foreach(KPathShape *shape, m_pointSelection.selectedShapes()) {
        QRectF roi = handleGrabRect(shape->documentToShape(event->point));
        KParameterShape * parameterShape = dynamic_cast<KParameterShape*>(shape);
        if (parameterShape && parameterShape->isParametricShape()) {
            int handleId = parameterShape->handleIdAt(roi);
            if (handleId != -1) {
                setCursor(m_moveCursor);
                emit statusTextChanged(i18n("Drag to move handle."));
                if (m_activeHandle)
                    m_activeHandle->repaint();
                delete m_activeHandle;

                //qDebug() << "handleId" << handleId;
                m_activeHandle = new ParameterHandle(this, parameterShape, handleId);
                m_activeHandle->repaint();
                return;
            }

        } else {
            QList<KPathPoint*> points = shape->pointsAt(roi);
            if (! points.empty()) {
                // find the nearest control point from all points within the roi
                KPathPoint * bestPoint = 0;
                KPathPoint::PointType bestPointType = KPathPoint::Node;
                qreal minDistance = HUGE_VAL;
                foreach(KPathPoint *p, points) {
                    // the node point must be hit if the point is not selected yet
                    if (! m_pointSelection.contains(p) && ! roi.contains(p->point()))
                        continue;

                    // check for the control points first as otherwise it is no longer
                    // possible to change the control points when they are the same as the point
                    if (p->activeControlPoint1() && roi.contains(p->controlPoint1())) {
                        qreal dist = squaredDistance(roi.center(), p->controlPoint1());
                        if (dist < minDistance) {
                            bestPoint = p;
                            bestPointType = KPathPoint::ControlPoint1;
                            minDistance = dist;
                        }
                    }

                    if (p->activeControlPoint2() && roi.contains(p->controlPoint2())) {
                        qreal dist = squaredDistance(roi.center(), p->controlPoint2());
                        if (dist < minDistance) {
                            bestPoint = p;
                            bestPointType = KPathPoint::ControlPoint2;
                            minDistance = dist;
                        }
                    }

                    // check the node point at last
                    qreal dist = squaredDistance(roi.center(), p->point());
                    if (dist < minDistance) {
                        bestPoint = p;
                        bestPointType = KPathPoint::Node;
                        minDistance = dist;
                    }
                }

                if (! bestPoint)
                    return;

                setCursor(m_moveCursor);
                if (bestPointType == KPathPoint::Node)
                    emit statusTextChanged(i18n("Drag to move point. Shift click to change point type."));
                else
                    emit statusTextChanged(i18n("Drag to move control point."));

                PointHandle *prev = dynamic_cast<PointHandle*>(m_activeHandle);
                if (prev && prev->activePoint() == bestPoint && prev->activePointType() == bestPointType)
                    return; // no change;

                if (m_activeHandle)
                    m_activeHandle->repaint();
                delete m_activeHandle;
                m_activeHandle = new PointHandle(this, bestPoint, bestPointType);
                m_activeHandle->repaint();
                return;
            }
        }
    }

    setCursor(m_selectCursor);
    if (m_activeHandle)
        m_activeHandle->repaint();
    delete m_activeHandle;
    m_activeHandle = 0;
    uint selectedPointCount = m_pointSelection.size();
    if (selectedPointCount == 0)
        emit statusTextChanged("");
    else if (selectedPointCount == 1)
        emit statusTextChanged(i18n("Press B to break path at selected point."));
    else
        emit statusTextChanged(i18n("Press B to break path at selected segments."));
}

void KPathTool::mouseReleaseEvent(KPointerEvent *event)
{
    Q_D(KToolBase);
    if (m_currentStrategy) {
        const bool hadNoSelection = !m_pointSelection.hasSelection();
        m_currentStrategy->finishInteraction(event->modifiers());
        QUndoCommand *command = m_currentStrategy->createCommand();
        if (command)
            d->canvas->addCommand(command);
        if (hadNoSelection && dynamic_cast<KPathPointRubberSelectStrategy*>(m_currentStrategy)
                && !m_pointSelection.hasSelection()) {
            // the click didn't do anything at all. Allow it to be used by others.
            event->ignore();
        }
        delete m_currentStrategy;
        m_currentStrategy = 0;

        if (m_toolOptionWidget) {
            if (m_pointSelection.selectedShapes().count() == 1)
                m_toolOptionWidget->setSelectedPath(m_pointSelection.selectedShapes().first());
            else
                m_toolOptionWidget->setSelectedPath(0);
        }
    }
}

void KPathTool::keyPressEvent(QKeyEvent *event)
{
    Q_D(KToolBase);
    if (m_currentStrategy) {
        switch (event->key()) {
        case Qt::Key_Control:
        case Qt::Key_Alt:
        case Qt::Key_Shift:
        case Qt::Key_Meta:
            if (! event->isAutoRepeat()) {
                m_currentStrategy->handleMouseMove(m_lastPoint, event->modifiers());
            }
            break;
        case Qt::Key_Escape:
            m_currentStrategy->cancelInteraction();
            delete m_currentStrategy;
            m_currentStrategy = 0;
            break;
        default:
            event->ignore();
            return;
        }
    } else {
        switch (event->key()) {
// TODO move these to the actions in the constructor.
        case Qt::Key_I: {
            int handleRadius = d->canvas->resourceManager()->handleRadius();
            if (event->modifiers() & Qt::ControlModifier)
                handleRadius--;
            else
                handleRadius++;
            d->canvas->resourceManager()->setHandleRadius(handleRadius);
            break;
        }
#ifndef NDEBUG
        case Qt::Key_D:
            if (m_pointSelection.objectCount() == 1) {
                QList<KPathPointData> selectedPoints = m_pointSelection.selectedPointsData();
                KPathShapePrivate *p = static_cast<KPathShapePrivate*>(selectedPoints[0].pathShape->priv());
                p->debugPath();
            }
            break;
#endif
        case Qt::Key_B:
            if (m_pointSelection.size() == 1)
                breakAtPoint();
            else if (m_pointSelection.size() >= 2)
                breakAtSegment();
            break;
        default:
            event->ignore();
            return;
        }
    }
    event->accept();
}

void KPathTool::keyReleaseEvent(QKeyEvent *event)
{
    if (m_currentStrategy) {
        switch (event->key()) {
        case Qt::Key_Control:
        case Qt::Key_Alt:
        case Qt::Key_Shift:
        case Qt::Key_Meta:
            if (! event->isAutoRepeat()) {
                m_currentStrategy->handleMouseMove(m_lastPoint, Qt::NoModifier);
            }
            break;
        default:
            break;
        }
    }
    event->accept();
}

void KPathTool::mouseDoubleClickEvent(KPointerEvent *event)
{
    Q_D(KToolBase);
    event->ignore();

    // check if we are doing something else at the moment
    if (m_currentStrategy)
        return;

    /*
    // TODO: use global click proximity once added to the canvas resource provider
    const int clickProximity = 5;

    // convert click proximity to point using the current zoom level
    QPointF clickOffset = d->canvas->viewConverter()->viewToDocument(QPointF(clickProximity, clickProximity));
    // the max allowed distance from a segment
    const qreal maxSquaredDistance = clickOffset.x()*clickOffset.x();

    KPathShape * clickedShape = 0;
    KPathPoint * clickedSegmentStart = 0;
    qreal clickedPointParam = 0.0;

    foreach(KPathShape *shape, m_pointSelection.selectedShapes()) {
        if (dynamic_cast<KParameterShape*>(shape))
            continue;

        // convert document point to shape coordinates
        QPointF point = shape->documentToShape(event->point);
        // our region of interest, i.e. a region around our mouse position
        QRectF roi(point - clickOffset, point + clickOffset);

        qreal minSqaredDistance = HUGE_VAL;
        // check all segments of this shape which intersect the region of interest
        QList<KPathSegment> segments = shape->segmentsAt(roi);
        foreach (const KPathSegment &s, segments) {
            qreal nearestPointParam = s.nearestPoint(point);
            QPointF nearestPoint = s.pointAt(nearestPointParam);
            QPointF diff = point - nearestPoint;
            qreal squaredDistance = diff.x()*diff.x() + diff.y()*diff.y();
            // are we within the allowed distance ?
            if (squaredDistance > maxSquaredDistance)
                continue;
            // are we closer to the last closest point ?
            if (squaredDistance < minSqaredDistance) {
                clickedShape = shape;
                clickedSegmentStart = s.first();
                clickedPointParam = nearestPointParam;
            }
        }
    }
    */

    KPathShape * clickedShape = 0;
    KPathPoint * clickedSegmentStart = 0;
    qreal clickedPointParam = 0.0;
    if (! segmentAtPoint(event->point, clickedShape, clickedSegmentStart, clickedPointParam))
        return;

    if (clickedShape && clickedSegmentStart) {
        QList<KPathPointData> segments;
        segments.append(KPathPointData(clickedShape, clickedShape->pathPointIndex(clickedSegmentStart)));
        KPathPointInsertCommand *cmd = new KPathPointInsertCommand(segments, clickedPointParam);
        d->canvas->addCommand(cmd);

        foreach (KPathPoint * p, cmd->insertedPoints()) {
            m_pointSelection.add(p, false);
        }
        updateActions();
        event->accept();
    }
}

bool KPathTool::segmentAtPoint(const QPointF &point, KPathShape* &shape, KPathPoint* &segmentStart, qreal &pointParam)
{
    Q_D(KToolBase);
    // TODO: use global click proximity once added to the canvas resource provider
    const int clickProximity = 5;

    // convert click proximity to point using the current zoom level
    QPointF clickOffset = d->canvas->viewConverter()->viewToDocument(QPointF(clickProximity, clickProximity));
    // the max allowed distance from a segment
    const qreal maxSquaredDistance = clickOffset.x()*clickOffset.x();

    KPathShape * clickedShape = 0;
    KPathPoint * clickedSegmentStart = 0;
    qreal clickedPointParam = 0.0;

    foreach(KPathShape *shape, m_pointSelection.selectedShapes()) {
        KParameterShape * parameterShape = dynamic_cast<KParameterShape*>(shape);
        if (parameterShape && parameterShape->isParametricShape())
            continue;

        // convert document point to shape coordinates
        QPointF p = shape->documentToShape(point);
        // our region of interest, i.e. a region around our mouse position
        QRectF roi(p - clickOffset, p + clickOffset);

        qreal minSqaredDistance = HUGE_VAL;
        // check all segments of this shape which intersect the region of interest
        QList<KPathSegment> segments = shape->segmentsAt(roi);
        foreach (const KPathSegment &s, segments) {
            qreal nearestPointParam = s.nearestPoint(p);
            QPointF nearestPoint = s.pointAt(nearestPointParam);
            QPointF diff = p - nearestPoint;
            qreal squaredDistance = diff.x()*diff.x() + diff.y()*diff.y();
            // are we within the allowed distance ?
            if (squaredDistance > maxSquaredDistance)
                continue;
            // are we closer to the last closest point ?
            if (squaredDistance < minSqaredDistance) {
                clickedShape = shape;
                clickedSegmentStart = s.first();
                clickedPointParam = nearestPointParam;
            }
        }
    }

    shape = clickedShape;
    segmentStart = clickedSegmentStart;
    pointParam = clickedPointParam;

    return (shape && segmentStart);
}

void KPathTool::activate(ToolActivation toolActivation, const QSet<KShape*> &shapes)
{
    Q_D(KToolBase);
    Q_UNUSED(toolActivation);
    // retrieve the actual global handle radius
    m_handleRadius = d->canvas->resourceManager()->handleRadius();
    d->canvas->snapGuide()->reset();

    repaintDecorations();
    QList<KPathShape*> selectedShapes;
    foreach(KShape *shape, shapes) {
        KPathShape *pathShape = dynamic_cast<KPathShape*>(shape);

        if (shape->isEditable() && pathShape) {
            // as the tool is just in activation repaintDecorations does not yet get called
            // so we need to use repaint of the tool and it is only needed to repaint the
            // current canvas
            repaint(pathShape->boundingRect());
            selectedShapes.append(pathShape);
        }
    }
    if (selectedShapes.isEmpty()) {
        emit done();
        return;
    }
    m_pointSelection.setSelectedShapes(selectedShapes);
    setCursor(m_selectCursor);
    connect(d->canvas->shapeManager()->selection(), SIGNAL(selectionChanged()), this, SLOT(activate()));
    updateOptionsWidget();
    updateActions();
}

void KPathTool::activate()
{
    Q_D(KToolBase);
    QSet<KShape*> shapes;
    foreach(KShape *shape, d->canvas->shapeManager()->selection()->selectedShapes()) {
        QSet<KShape*> delegates = shape->toolDelegates();
        if (delegates.isEmpty()) {
            shapes << shape;
        } else {
            shapes += delegates;
        }
    }
    activate(DefaultActivation, shapes);
}

void KPathTool::updateOptionsWidget()
{
    if (m_toolOptionWidget == 0)
        return;
    PathToolOptionWidget::Type type = PathToolOptionWidget::PlainType;
    QList<KPathShape*> selectedShapes = m_pointSelection.selectedShapes();
    foreach(KPathShape *shape, selectedShapes) {
        KParameterShape *parameterShape = dynamic_cast<KParameterShape*>(shape);
        if (parameterShape && parameterShape->isParametricShape()) {
            type = PathToolOptionWidget::ParametricType;
            m_toolOptionWidget->setSelectedPath(shape);
            break;
        }
    }
    m_toolOptionWidget->setSelectionType(type);
    if (type == PathToolOptionWidget::ParametricType)
        m_toolOptionWidget->setSelectedPath(selectedShapes.first());
}

void KPathTool::updateActions()
{
    const bool hasPointsSelected = m_pointSelection.hasSelection();
    m_actionPathPointCorner->setEnabled(hasPointsSelected);
    m_actionPathPointSmooth->setEnabled(hasPointsSelected);
    m_actionPathPointSymmetric->setEnabled(hasPointsSelected);
    m_actionRemovePoint->setEnabled(hasPointsSelected);
    m_actionBreakPoint->setEnabled(hasPointsSelected);
    m_actionCurvePoint->setEnabled(hasPointsSelected);
    m_actionLinePoint->setEnabled(hasPointsSelected);

    bool hasSegmentsSelected = false;
    if (hasPointsSelected && m_pointSelection.size() > 1)
        hasSegmentsSelected = !m_pointSelection.selectedSegmentsData().isEmpty();
    m_actionAddPoint->setEnabled(hasSegmentsSelected);
    m_actionLineSegment->setEnabled(hasSegmentsSelected);
    m_actionCurveSegment->setEnabled(hasSegmentsSelected);

    const uint objectCount = m_pointSelection.objectCount();
    const uint pointCount = m_pointSelection.size();
    m_actionBreakSegment->setEnabled(objectCount == 1 && pointCount == 2);
    m_actionJoinSegment->setEnabled(objectCount == 1 && pointCount == 2);
    m_actionMergePoints->setEnabled(objectCount == 1 && pointCount == 2);
}

void KPathTool::deactivate()
{
    Q_D(KToolBase);
    disconnect(d->canvas->shapeManager()->selection(), SIGNAL(selectionChanged()), this, SLOT(activate()));
    m_pointSelection.clear();
    m_pointSelection.setSelectedShapes(QList<KPathShape*>());
    delete m_activeHandle;
    m_activeHandle = 0;
    delete m_currentStrategy;
    m_currentStrategy = 0;
    d->canvas->snapGuide()->reset();
}

void KPathTool::resourceChanged(int key, const QVariant & res)
{
    if (key == KCanvasResource::HandleRadius) {
        int oldHandleRadius = m_handleRadius;

        m_handleRadius = res.toUInt();

        // repaint with the bigger of old and new handle radius
        int maxRadius = qMax(m_handleRadius, oldHandleRadius);
        foreach(KPathShape *shape, m_pointSelection.selectedShapes()) {
            QRectF controlPointRect = shape->absoluteTransformation(0).map(shape->outline()).controlPointRect();
            repaint(controlPointRect.adjusted(-maxRadius, -maxRadius, maxRadius, maxRadius));
        }
    }
}

void KPathTool::pointSelectionChanged()
{
    Q_D(KToolBase);
    updateActions();
    d->canvas->snapGuide()->setIgnoredPathPoints(m_pointSelection.selectedPoints().toList());
    emit selectionChanged(m_pointSelection.hasSelection());
}

void KPathTool::repaint(const QRectF &repaintRect)
{
    Q_D(KToolBase);
    //kDebug(30006) <<"KPathTool::repaint(" << repaintRect <<")" << m_handleRadius;
    // widen border to take antialiasing into account
    qreal radius = m_handleRadius + 1;
    d->canvas->updateCanvas(repaintRect.adjusted(-radius, -radius, radius, radius));
}

void KPathTool::deleteSelection()
{
    removePoints();
}

KToolSelection * KPathTool::selection()
{
    return &m_pointSelection;
}

#include <KPathTool_p.moc>
