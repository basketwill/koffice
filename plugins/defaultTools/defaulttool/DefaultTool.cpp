/* This file is part of the KDE project

   Copyright (C) 2006-2008 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006-2011 Thomas Zander <zander@kde.org>
   Copyright (C) 2008-2009 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2008 Casper Boemann <cbr@boemann.dk>

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
 * Boston, MA 02110-1301, USA.
*/

#include "DefaultTool.h"
#include "DefaultToolWidget.h"
#include "DefaultToolArrangeWidget.h"
#include "ConnectionChangeStrategy.h"
#include "SelectionDecorator.h"
#include "ShapeMoveStrategy.h"
#include "ShapeRotateStrategy.h"
#include "ShapeShearStrategy.h"
#include "ShapeResizeStrategy.h"
#include "guidestool/GuidesTool.h"
#include "guidestool/GuidesToolFactory.h" // for the ID

#include <KPointerEvent.h>
#include <KShapeConnection.h>
#include <KToolSelection.h>
#include <KToolManager.h>
#include <KToolRegistry.h>
#include <KGuidesData.h>
#include <KShapeController.h>
#include <KShapeManager.h>
#include <KShapeGroup.h>
#include <KShapePaste.h>
#include <KShapeOdfSaveHelper.h>
#include <KDrag.h>
#include <KCanvasBase.h>
#include <KResourceManager.h>
#include <KShapeRubberSelectStrategy.h>
#include <commands/KShapeMoveCommand.h>
#include <commands/KShapeGroupCommand.h>
#include <commands/KShapeUngroupCommand.h>
#include <KSnapGuide.h>

#include <KDebug>

#include <KAction>
#include <QClipboard>
#include <kstandarddirs.h>

#include <math.h>

#define HANDLE_DISTANCE 10

class SelectionHandler : public KToolSelection
{
public:
    SelectionHandler(DefaultTool *parent)
        : KToolSelection(parent), m_selection(parent->koSelection())
    {
        Q_ASSERT(m_selection);
    }

    bool hasSelection() {
        return m_selection->count();
    }

private:
    KShapeSelection *m_selection;
};

class DefaultTool::GuideLine
{
public:
    GuideLine()
        : m_orientation(Qt::Horizontal), m_index(0), m_valid(false), m_selected(false)
    {
    }
    GuideLine(Qt::Orientation orientation, uint index)
        : m_orientation(orientation), m_index(index), m_valid(true), m_selected(false)
    {
    }

    bool isValid() const
    {
        return m_valid;
    }
    bool isSelected() const
    {
        return m_selected;
    }
    void select()
    {
        m_selected = true;
    }

    uint index() const
    {
        return m_index;
    }
    Qt::Orientation orientation() const
    {
        return m_orientation;
    }
private:
    Qt::Orientation m_orientation;
    uint m_index;
    bool m_valid;
    bool m_selected;
};


DefaultTool::DefaultTool(KCanvasBase *canvas)
    : KInteractionTool(canvas),
    m_lastHandle(KFlake::NoHandle),
    m_hotPosition(KFlake::TopLeftCorner),
    m_mouseWasInsideHandles(false),
    m_moveCommand(0),
    m_selectionHandler(new SelectionHandler(this)),
    m_guideLine(new GuideLine())
{
    setupActions();

    QPixmap rotatePixmap, shearPixmap;
    rotatePixmap.load(KStandardDirs::locate("data", "koffice/icons/rotate.png"));
    shearPixmap.load(KStandardDirs::locate("data", "koffice/icons/shear.png"));

    m_rotateCursors[0] = QCursor(rotatePixmap.transformed(QTransform().rotate(45)));
    m_rotateCursors[1] = QCursor(rotatePixmap.transformed(QTransform().rotate(90)));
    m_rotateCursors[2] = QCursor(rotatePixmap.transformed(QTransform().rotate(135)));
    m_rotateCursors[3] = QCursor(rotatePixmap.transformed(QTransform().rotate(180)));
    m_rotateCursors[4] = QCursor(rotatePixmap.transformed(QTransform().rotate(225)));
    m_rotateCursors[5] = QCursor(rotatePixmap.transformed(QTransform().rotate(270)));
    m_rotateCursors[6] = QCursor(rotatePixmap.transformed(QTransform().rotate(315)));
    m_rotateCursors[7] = QCursor(rotatePixmap);
/*
    m_rotateCursors[0] = QCursor(Qt::RotateNCursor);
    m_rotateCursors[1] = QCursor(Qt::RotateNECursor);
    m_rotateCursors[2] = QCursor(Qt::RotateECursor);
    m_rotateCursors[3] = QCursor(Qt::RotateSECursor);
    m_rotateCursors[4] = QCursor(Qt::RotateSCursor);
    m_rotateCursors[5] = QCursor(Qt::RotateSWCursor);
    m_rotateCursors[6] = QCursor(Qt::RotateWCursor);
    m_rotateCursors[7] = QCursor(Qt::RotateNWCursor);
*/
    m_shearCursors[0] = QCursor(shearPixmap);
    m_shearCursors[1] = QCursor(shearPixmap.transformed(QTransform().rotate(45)));
    m_shearCursors[2] = QCursor(shearPixmap.transformed(QTransform().rotate(90)));
    m_shearCursors[3] = QCursor(shearPixmap.transformed(QTransform().rotate(135)));
    m_shearCursors[4] = QCursor(shearPixmap.transformed(QTransform().rotate(180)));
    m_shearCursors[5] = QCursor(shearPixmap.transformed(QTransform().rotate(225)));
    m_shearCursors[6] = QCursor(shearPixmap.transformed(QTransform().rotate(270)));
    m_shearCursors[7] = QCursor(shearPixmap.transformed(QTransform().rotate(315)));
    m_sizeCursors[0] = Qt::SizeVerCursor;
    m_sizeCursors[1] = Qt::SizeBDiagCursor;
    m_sizeCursors[2] = Qt::SizeHorCursor;
    m_sizeCursors[3] = Qt::SizeFDiagCursor;
    m_sizeCursors[4] = Qt::SizeVerCursor;
    m_sizeCursors[5] = Qt::SizeBDiagCursor;
    m_sizeCursors[6] = Qt::SizeHorCursor;
    m_sizeCursors[7] = Qt::SizeFDiagCursor;

    KShapeManager * manager = canvas->shapeManager();
    connect(manager, SIGNAL(selectionChanged()), this, SLOT(updateActions()));

    setFlags(ToolDoesntAutoScroll | ToolHandleKeyEvents | ToolMouseTracking);

    QStringList list;
    list << KOdf::mimeType(KOdf::TextDocument);
    setSupportedPasteMimeTypes(list);
}

DefaultTool::~DefaultTool()
{
    delete m_guideLine;
}

void DefaultTool::setupActions()
{
    KAction* actionBringToFront = new KAction(KIcon("object-order-front-koffice"),
                                               i18n("Bring to &Front"), this);
    addAction("object_order_front", actionBringToFront);
    actionBringToFront->setShortcut(QKeySequence("Ctrl+Shift+]"));
    connect(actionBringToFront, SIGNAL(triggered()), this, SLOT(selectionBringToFront()));

    KAction* actionRaise = new KAction(KIcon("object-order-raise-koffice"), i18n("&Raise"), this);
    addAction("object_order_raise", actionRaise);
    actionRaise->setShortcut(QKeySequence("Ctrl+]"));
    connect(actionRaise, SIGNAL(triggered()), this, SLOT(selectionMoveUp()));

    KAction* actionLower = new KAction(KIcon("object-order-lower-koffice"), i18n("&Lower"), this);
    addAction("object_order_lower", actionLower);
    actionLower->setShortcut(QKeySequence("Ctrl+["));
    connect(actionLower, SIGNAL(triggered()), this, SLOT(selectionMoveDown()));

    KAction* actionSendToBack = new KAction(KIcon("object-order-back-koffice"),
                                             i18n("Send to &Back"), this);
    addAction("object_order_back", actionSendToBack);
    actionSendToBack->setShortcut(QKeySequence("Ctrl+Shift+["));
    connect(actionSendToBack, SIGNAL(triggered()), this, SLOT(selectionSendToBack()));

    KAction* actionAlignLeft = new KAction(KIcon("object-align-horizontal-left-koffice"),
                                            i18n("Align Left"), this);
    addAction("object_align_horizontal_left", actionAlignLeft);
    connect(actionAlignLeft, SIGNAL(triggered()), this, SLOT(selectionAlignHorizontalLeft()));

    KAction* actionAlignCenter = new KAction(KIcon("object-align-horizontal-center-koffice"),
                                              i18n("Horizontally Center"), this);
    addAction("object_align_horizontal_center", actionAlignCenter);
    connect(actionAlignCenter, SIGNAL(triggered()), this, SLOT(selectionAlignHorizontalCenter()));

    KAction* actionAlignRight = new KAction(KIcon("object-align-horizontal-right-koffice"),
                                             i18n("Align Right"), this);
    addAction("object_align_horizontal_right", actionAlignRight);
    connect(actionAlignRight, SIGNAL(triggered()), this, SLOT(selectionAlignHorizontalRight()));

    KAction* actionAlignTop = new KAction(KIcon("object-align-vertical-top-koffice"), i18n("Align Top"), this);
    addAction("object_align_vertical_top", actionAlignTop);
    connect(actionAlignTop, SIGNAL(triggered()), this, SLOT(selectionAlignVerticalTop()));

    KAction* actionAlignMiddle = new KAction(KIcon("object-align-vertical-center-koffice"),
                                              i18n("Vertically Center"), this);
    addAction("object_align_vertical_center", actionAlignMiddle);
    connect(actionAlignMiddle, SIGNAL(triggered()), this, SLOT(selectionAlignVerticalCenter()));

    KAction* actionAlignBottom = new KAction(KIcon("object-align-vertical-bottom-koffice"),
                                              i18n("Align Bottom"), this);
    addAction("object_align_vertical_bottom", actionAlignBottom);
    connect(actionAlignBottom, SIGNAL(triggered()), this, SLOT(selectionAlignVerticalBottom()));

    KAction* actionGroupBottom = new KAction(KIcon("object-group-koffice"),
                                              i18n("Group"), this);
    addAction("object_group", actionGroupBottom);
    connect(actionGroupBottom, SIGNAL(triggered()), this, SLOT(selectionGroup()));

    KAction* actionUngroupBottom = new KAction(KIcon("object-ungroup-koffice"),
                                                i18n("Ungroup"), this);
    addAction("object_ungroup", actionUngroupBottom);
    connect(actionUngroupBottom, SIGNAL(triggered()), this, SLOT(selectionUngroup()));
}

qreal DefaultTool::rotationOfHandle(KFlake::SelectionHandle handle, bool useEdgeRotation)
{
    QPointF selectionCenter = koSelection()->absolutePosition();
    QPointF direction;

    switch (handle) {
    case KFlake::TopMiddleHandle:
        if (useEdgeRotation) {
            direction = koSelection()->absolutePosition(KFlake::TopRightCorner)
                - koSelection()->absolutePosition(KFlake::TopLeftCorner);
        } else {
            QPointF handlePosition = koSelection()->absolutePosition(KFlake::TopLeftCorner);
            handlePosition += 0.5 * (koSelection()->absolutePosition(KFlake::TopRightCorner) - handlePosition);
            direction = handlePosition - selectionCenter;
        }
        break;
    case KFlake::TopRightHandle:
        direction = koSelection()->absolutePosition(KFlake::TopRightCorner) - selectionCenter;
        break;
    case KFlake::RightMiddleHandle:
        if (useEdgeRotation) {
            direction = koSelection()->absolutePosition(KFlake::BottomRightCorner)
                    - koSelection()->absolutePosition(KFlake::TopRightCorner);
        } else {
            QPointF handlePosition = koSelection()->absolutePosition(KFlake::TopRightCorner);
            handlePosition += 0.5 * (koSelection()->absolutePosition(KFlake::BottomRightCorner) - handlePosition);
            direction = handlePosition - selectionCenter;
        }
        break;
    case KFlake::BottomRightHandle:
        direction = koSelection()->absolutePosition(KFlake::BottomRightCorner) - selectionCenter;
        break;
    case KFlake::BottomMiddleHandle:
        if (useEdgeRotation) {
            direction = koSelection()->absolutePosition(KFlake::BottomLeftCorner)
                    - koSelection()->absolutePosition(KFlake::BottomRightCorner);
        } else {
            QPointF handlePosition = koSelection()->absolutePosition(KFlake::BottomLeftCorner);
            handlePosition += 0.5 * (koSelection()->absolutePosition(KFlake::BottomRightCorner) - handlePosition);
            direction = handlePosition - selectionCenter;
        }
        break;
    case KFlake::BottomLeftHandle:
        direction = koSelection()->absolutePosition(KFlake::BottomLeftCorner) - selectionCenter;
        break;
    case KFlake::LeftMiddleHandle:
        if (useEdgeRotation) {
            direction = koSelection()->absolutePosition(KFlake::TopLeftCorner)
                    - koSelection()->absolutePosition(KFlake::BottomLeftCorner);
        } else {
            QPointF handlePosition = koSelection()->absolutePosition(KFlake::TopLeftCorner);
            handlePosition += 0.5 * (koSelection()->absolutePosition(KFlake::BottomLeftCorner) - handlePosition);
            direction = handlePosition - selectionCenter;
        }
        break;
    case KFlake::TopLeftHandle:
        direction = koSelection()->absolutePosition(KFlake::TopLeftCorner) - selectionCenter;
        break;
    case KFlake::NoHandle:
        return 0.0;
        break;
    }

    qreal rotation = atan2(direction.y(), direction.x()) * 180.0 / M_PI;

    switch (handle) {
    case KFlake::TopMiddleHandle:
        if (useEdgeRotation)
            rotation -= 0.0;
        else
            rotation -= 270.0;
        break;
    case KFlake::TopRightHandle:
        rotation -= 315.0;
        break;
    case KFlake::RightMiddleHandle:
        if (useEdgeRotation)
            rotation -= 90.0;
        else
            rotation -= 0.0;
        break;
    case KFlake::BottomRightHandle:
        rotation -= 45.0;
        break;
    case KFlake::BottomMiddleHandle:
        if (useEdgeRotation)
            rotation -= 180.0;
        else
            rotation -= 90.0;
        break;
    case KFlake::BottomLeftHandle:
        rotation -= 135.0;
        break;
    case KFlake::LeftMiddleHandle:
        if (useEdgeRotation)
            rotation -= 270.0;
        else
            rotation -= 180.0;
        break;
    case KFlake::TopLeftHandle:
        rotation -= 225.0;
        break;
    case KFlake::NoHandle:
        break;
    }

    if (rotation < 0.0)
        rotation += 360.0;

    return rotation;
}

void DefaultTool::updateCursor()
{
    QCursor cursor = Qt::ArrowCursor;

    QString statusText;

    if (koSelection()->count() > 0) { // has a selection
        bool editable=editableShapesCount(koSelection()->selectedShapes(KFlake::StrippedSelection));

        if (!m_mouseWasInsideHandles) {
            m_angle = rotationOfHandle(m_lastHandle, true);
            int rotOctant = 8 + int(8.5 + m_angle / 45);

            bool rotateHandle = false;
            bool shearHandle = false;
            switch(m_lastHandle) {
            case KFlake::TopMiddleHandle:
                cursor = m_shearCursors[(0 +rotOctant)%8];
                shearHandle = true;
                break;
            case KFlake::TopRightHandle:
                cursor = m_rotateCursors[(1 +rotOctant)%8];
                rotateHandle = true;
                break;
            case KFlake::RightMiddleHandle:
                cursor = m_shearCursors[(2 +rotOctant)%8];
                shearHandle = true;
                break;
            case KFlake::BottomRightHandle:
                cursor = m_rotateCursors[(3 +rotOctant)%8];
                rotateHandle = true;
                break;
            case KFlake::BottomMiddleHandle:
                cursor = m_shearCursors[(4 +rotOctant)%8];
                shearHandle = true;
                break;
            case KFlake::BottomLeftHandle:
                cursor = m_rotateCursors[(5 +rotOctant)%8];
                rotateHandle = true;
                break;
            case KFlake::LeftMiddleHandle:
                cursor = m_shearCursors[(6 +rotOctant)%8];
                shearHandle = true;
                break;
            case KFlake::TopLeftHandle:
                cursor = m_rotateCursors[(7 +rotOctant)%8];
                rotateHandle = true;
                break;
            case KFlake::NoHandle:
                if (m_guideLine->isValid()) {
                    cursor = m_guideLine->orientation() == Qt::Horizontal ? Qt::SizeVerCursor : Qt::SizeHorCursor;
                    statusText = i18n("Click and drag to move guide line.");
                }
                else
                    cursor = Qt::ArrowCursor;
                break;
            }
            if (rotateHandle)
                statusText = i18n("Left click rotates around center, right click around highlighted position.");
            if (shearHandle)
                statusText = i18n("Click and drag to shear selection.");
        }
        else {
            statusText = i18n("Click and drag to resize selection.");
            m_angle = rotationOfHandle(m_lastHandle, false);
            int rotOctant = 8 + int(8.5 + m_angle / 45);
            bool cornerHandle = false;
            switch(m_lastHandle) {
            case KFlake::TopMiddleHandle:
                cursor = m_sizeCursors[(0 +rotOctant)%8];
                break;
            case KFlake::TopRightHandle:
                cursor = m_sizeCursors[(1 +rotOctant)%8];
                cornerHandle = true;
                break;
            case KFlake::RightMiddleHandle:
                cursor = m_sizeCursors[(2 +rotOctant)%8];
                break;
            case KFlake::BottomRightHandle:
                cursor = m_sizeCursors[(3 +rotOctant)%8];
                cornerHandle = true;
                break;
            case KFlake::BottomMiddleHandle:
                cursor = m_sizeCursors[(4 +rotOctant)%8];
                break;
            case KFlake::BottomLeftHandle:
                cursor = m_sizeCursors[(5 +rotOctant)%8];
                cornerHandle = true;
                break;
            case KFlake::LeftMiddleHandle:
                cursor = m_sizeCursors[(6 +rotOctant)%8];
                break;
            case KFlake::TopLeftHandle:
                cursor = m_sizeCursors[(7 +rotOctant)%8];
                cornerHandle = true;
                break;
            case KFlake::NoHandle:
                cursor = Qt::SizeAllCursor;
                statusText = i18n("Click and drag to move selection.");
                break;
            }
            if (cornerHandle)
                statusText = i18n("Click and drag to resize selection. Middle click to set highlighted position.");
        }
        if (!editable)
            cursor = Qt::ArrowCursor;
    }
    else {
        if (m_guideLine->isValid()) {
            cursor = m_guideLine->orientation() == Qt::Horizontal ? Qt::SizeVerCursor : Qt::SizeHorCursor;
            statusText = i18n("Click and drag to move guide line.");
        }
    }
    setCursor(cursor);
    if (currentStrategy() == 0)
        emit statusTextChanged(statusText);
}

void DefaultTool::paint(QPainter &painter, const KViewConverter &converter)
{
    KInteractionTool::paint(painter, converter);
    if (currentStrategy() == 0 && koSelection()->count() > 0) {
        SelectionDecorator decorator(m_mouseWasInsideHandles ? m_lastHandle : KFlake::NoHandle,
                 true, true);
        decorator.setSelection(koSelection());
        decorator.setHandleRadius(canvas()->resourceManager()->handleRadius());
        decorator.setHotPosition(m_hotPosition);
        decorator.paint(painter, converter);
    }

    painter.save();
    KShape::applyConversion(painter, converter);
    canvas()->snapGuide()->paint(painter, converter);
    painter.restore();

    painter.save();
    painter.setPen(QPen(Qt::black)); // TODO make configurable
    painter.setBrush(QBrush(Qt::red));
    foreach (KShapeConnection *connection, m_selectedConnections) {
        QPointF center = converter.documentToView(connection->startPoint());
        for (int i = 0 ; i < 2; ++i) {
            painter.drawEllipse(center, 3, 3); // TODO is that 3 correct?
            center = converter.documentToView(connection->endPoint());
        }
    }
    painter.restore();
}

void DefaultTool::mousePressEvent(KPointerEvent *event)
{
    KInteractionTool::mousePressEvent(event);
    updateCursor();
}

void DefaultTool::mouseMoveEvent(KPointerEvent *event)
{
    KInteractionTool::mouseMoveEvent(event);
    if (currentStrategy() == 0 && koSelection()->count() > 0) {
        QRectF bound = handlesSize();
        if (bound.contains(event->point)) {
            bool inside;
            KFlake::SelectionHandle newDirection = handleAt(event->point, &inside);
            if (inside != m_mouseWasInsideHandles || m_lastHandle != newDirection) {
                m_lastHandle = newDirection;
                m_mouseWasInsideHandles = inside;
                //repaintDecorations();
            }
        } else {
            /*if (m_lastHandle != KFlake::NoHandle)
                repaintDecorations(); */
            m_lastHandle = KFlake::NoHandle;
            m_mouseWasInsideHandles = false;

            if (m_guideLine->isSelected()) {
                GuidesTool *guidesTool = dynamic_cast<GuidesTool*>(KToolManager::instance()->toolById(canvas(), GuidesToolId));
                if (guidesTool) {
                    guidesTool->moveGuideLine(m_guideLine->orientation(), m_guideLine->index());
                    activateTemporary(guidesTool->toolId());
                }
            } else {
                selectGuideAtPosition(event->point);
            }
        }
    } else {
        if (m_guideLine->isSelected()) {
            GuidesTool *guidesTool = dynamic_cast<GuidesTool*>(KToolManager::instance()->toolById(canvas(), GuidesToolId));
            if (guidesTool) {
                guidesTool->moveGuideLine(m_guideLine->orientation(), m_guideLine->index());
                activateTemporary(guidesTool->toolId());
            }
        } else {
            selectGuideAtPosition(event->point);
        }
    }

    updateCursor();
}

void DefaultTool::selectGuideAtPosition(const QPointF &position)
{
    int index = -1;
    Qt::Orientation orientation = Qt::Horizontal;

    // check if we are on a guide line
    KGuidesData * guidesData = canvas()->guidesData();
    if (guidesData && guidesData->showGuideLines()) {
        qreal grabSensitivity = canvas()->resourceManager()->grabSensitivity();
        qreal minDistance = canvas()->viewConverter()->viewToDocumentX(grabSensitivity);
        uint i = 0;
        foreach (qreal guidePos, guidesData->horizontalGuideLines()) {
            qreal distance = qAbs(guidePos - position.y());
            if (distance < minDistance) {
                orientation = Qt::Horizontal;
                index = i;
                minDistance = distance;
            }
            i++;
        }
        i = 0;
        foreach (qreal guidePos, guidesData->verticalGuideLines())
        {
            qreal distance = qAbs(guidePos - position.x());
            if (distance < minDistance) {
                orientation = Qt::Vertical;
                index = i;
                minDistance = distance;
            }
            i++;
        }
    }

    delete m_guideLine;
    if (index >= 0)
        m_guideLine = new GuideLine(orientation, index);
    else
        m_guideLine = new GuideLine();
}

QRectF DefaultTool::handlesSize()
{
    QRectF bound = koSelection()->boundingRect();

    // repaint connections too
    foreach (KShapeConnection *connection, m_selectedConnections) {
        bound = bound.unite(connection->boundingRect());
    }

    // expansion Border
    if (!canvas() || !canvas()->viewConverter()) return bound;

    QPointF border = canvas()->viewConverter()->viewToDocument(QPointF(HANDLE_DISTANCE, HANDLE_DISTANCE));
    bound.adjust(-border.x(), -border.y(), border.x(), border.y());
    return bound;
}

void DefaultTool::mouseReleaseEvent(KPointerEvent *event)
{
    KInteractionTool::mouseReleaseEvent(event);
    updateCursor();
}

void DefaultTool::mouseDoubleClickEvent(KPointerEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;
    QList<KShape*> shapes;
    foreach(KShape *shape, koSelection()->selectedShapes()) {
        if (shape->boundingRect().contains(event->point) && // first 'cheap' check
                shape->outline().contains(event->point)) // this is more expensive but weeds out the almost hits
            shapes.append(shape);
    }
    if (shapes.count() == 0) { // nothing in the selection was clicked on.
        KShape *shape = canvas()->shapeManager()->shapeAt (event->point, KFlake::ShapeOnTop);
        if (shape) {
            shapes.append(shape);
        } else if (m_guideLine->isSelected()) {
            GuidesTool *guidesTool = dynamic_cast<GuidesTool*>(KToolManager::instance()->toolById(canvas(), GuidesToolId));
            if (guidesTool) {
                guidesTool->editGuideLine(m_guideLine->orientation(), m_guideLine->index());
                activateTool(guidesTool->toolId());
                return;
            }
        }
    }

    QList<KShape*> shapes2;
    foreach (KShape *shape, shapes) {
        QSet<KShape*> delegates = shape->toolDelegates();
        if (delegates.isEmpty()) {
            shapes2.append(shape);
        } else {
            foreach (KShape *delegatedShape, delegates) {
                shapes2.append(delegatedShape);
            }
        }
    }


    KToolManager::instance()->switchToolRequested(
            KToolManager::instance()->preferredToolForSelection(shapes2));
}

bool DefaultTool::moveSelection(int direction, Qt::KeyboardModifiers modifiers)
{
    qreal x=0.0, y=0.0;
    if (direction == Qt::Key_Left)
        x = -5;
    else if (direction == Qt::Key_Right)
        x = 5;
    else if (direction == Qt::Key_Up)
        y = -5;
    else if (direction == Qt::Key_Down)
        y = 5;

    if (x != 0.0 || y != 0.0) { // actually move
        if ((modifiers & Qt::ShiftModifier) != 0) {
            x *= 10;
            y *= 10;
        } else if ((modifiers & Qt::AltModifier) != 0) { // more precise
            x /= 5;
            y /= 5;
        }

        QList<QPointF> prevPos;
        QList<QPointF> newPos;
        QList<KShape*> shapes;
        foreach(KShape* shape, koSelection()->selectedShapes(KFlake::TopLevelSelection)) {
            if (shape->isGeometryProtected())
                continue;
            shapes.append(shape);
            QPointF p = shape->position();
            prevPos.append(p);
            p.setX(p.x() + x);
            p.setY(p.y() + y);
            newPos.append(p);
        }
        if (shapes.count() > 0) {
            // use a timeout to make sure we don't reuse a command possibly deleted by the commandHistory
            if (m_lastUsedMoveCommand.msecsTo(QTime::currentTime()) > 5000)
                m_moveCommand = 0;
            if (m_moveCommand) { // alter previous instead of creating new one.
                m_moveCommand->setNewPositions(newPos);
                m_moveCommand->redo();
            } else {
                m_moveCommand = new KShapeMoveCommand(shapes, prevPos, newPos);
                canvas()->addCommand(m_moveCommand);
            }
            m_lastUsedMoveCommand = QTime::currentTime();
            return true;
        }
    }
    return false;
}

void DefaultTool::keyPressEvent(QKeyEvent *event)
{
    KInteractionTool::keyPressEvent(event);
    if (currentStrategy() == 0) {
        switch (event->key()) {
        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_Up:
        case Qt::Key_Down:
            if (moveSelection(event->key(), event->modifiers()))
                event->accept();
            break;
        case Qt::Key_1:
        case Qt::Key_2:
        case Qt::Key_3:
        case Qt::Key_4:
        case Qt::Key_5:
            canvas()->resourceManager()->setResource(HotPosition, event->key()-Qt::Key_1);
            event->accept();
            break;
        default:
            return;
        }
    }
}

void DefaultTool::repaintDecorations()
{
    Q_ASSERT(koSelection());
    if (koSelection()->count() > 0 || !m_selectedConnections.isEmpty())
        canvas()->updateCanvas(handlesSize());
}

void DefaultTool::copy() const
{
    QList<KShape *> shapes = canvas()->shapeManager()->selection()->selectedShapes(KFlake::TopLevelSelection);
    if (!shapes.empty()) {
        KShapeOdfSaveHelper saveHelper(shapes);
        KDrag drag;
        drag.setOdf(KOdf::mimeType(KOdf::TextDocument), saveHelper);
        drag.addToClipboard();
    }
}

void DefaultTool::deleteSelection()
{
    QList<KShape *> shapes;
    foreach (KShape *s, canvas()->shapeManager()->selection()->selectedShapes(KFlake::TopLevelSelection)) {
        if (s->isGeometryProtected())
            continue;
        shapes << s;
    }
    if (!shapes.empty()) {
        canvas()->addCommand(canvas()->shapeController()->removeShapes(shapes));
    }
}

bool DefaultTool::paste()
{
    const QMimeData * data = QApplication::clipboard()->mimeData();

    bool success = false;
    if (data->hasFormat(KOdf::mimeType(KOdf::TextDocument))) {
        KShapeManager * shapeManager = canvas()->shapeManager();
        KShapePaste paste(canvas(), shapeManager->selection()->activeLayer());
        success = paste.paste(KOdf::TextDocument, data);
        if (success) {
            shapeManager->selection()->deselectAll();
            foreach(KShape *shape, paste.pastedShapes()) {
                shapeManager->selection()->select(shape);
            }
        }
    }
    return success;
}

KShapeSelection *DefaultTool::koSelection()
{
    Q_ASSERT(canvas());
    Q_ASSERT(canvas()->shapeManager());
    return canvas()->shapeManager()->selection();
}

KFlake::SelectionHandle DefaultTool::handleAt(const QPointF &point, bool *innerHandleMeaning)
{
    // check for handles in this order; meaning that when handles overlap the one on top is chosen
    static const KFlake::SelectionHandle handleOrder[] = {
        KFlake::BottomRightHandle,
        KFlake::TopLeftHandle,
        KFlake::BottomLeftHandle,
        KFlake::TopRightHandle,
        KFlake::BottomMiddleHandle,
        KFlake::RightMiddleHandle,
        KFlake::LeftMiddleHandle,
        KFlake::TopMiddleHandle,
        KFlake::NoHandle
    };

    if (koSelection()->count() == 0)
        return KFlake::NoHandle;

    recalcSelectionBox();
    const KViewConverter *converter = canvas()->viewConverter();
    if (!converter) return KFlake::NoHandle;

    if (innerHandleMeaning != 0)
    {
        QPainterPath path;
        path.addPolygon(m_selectionOutline);
        *innerHandleMeaning = path.contains(point) || path.intersects(handlePaintRect(point));
    }
    for (int i = 0; i < KFlake::NoHandle; ++i) {
        KFlake::SelectionHandle handle = handleOrder[i];
        QPointF pt = converter->documentToView(point) - converter->documentToView(m_selectionBox[handle]);

        // if just inside the outline
        if (qAbs(pt.x()) < HANDLE_DISTANCE &&
                qAbs(pt.y()) < HANDLE_DISTANCE) {
            if (innerHandleMeaning != 0)
            {
                if (qAbs(pt.x()) < 4 && qAbs(pt.y()) < 4)
                    *innerHandleMeaning = true;
            }
            return handle;
        }
    }
    return KFlake::NoHandle;
}

void DefaultTool::recalcSelectionBox()
{
    if (koSelection()->count()==0)
        return;

    if (koSelection()->count()>1) {
        QTransform matrix = koSelection()->absoluteTransformation(0);
        m_selectionOutline = matrix.map(QPolygonF(QRectF(QPointF(0, 0), koSelection()->size())));
        m_angle = 0.0; //koSelection()->rotation();
    } else {
        QTransform matrix = koSelection()->firstSelectedShape()->absoluteTransformation(0);
        m_selectionOutline = matrix.map(QPolygonF(QRectF(QPointF(0, 0), koSelection()->firstSelectedShape()->size())));
        m_angle = 0.0; //koSelection()->firstSelectedShape()->rotation();
    }
    QPolygonF outline = m_selectionOutline; //shorter name in the following :)
    m_selectionBox[KFlake::TopMiddleHandle] = (outline.value(0)+outline.value(1))/2;
    m_selectionBox[KFlake::TopRightHandle] = outline.value(1);
    m_selectionBox[KFlake::RightMiddleHandle] = (outline.value(1)+outline.value(2))/2;
    m_selectionBox[KFlake::BottomRightHandle] = outline.value(2);
    m_selectionBox[KFlake::BottomMiddleHandle] = (outline.value(2)+outline.value(3))/2;
    m_selectionBox[KFlake::BottomLeftHandle] = outline.value(3);
    m_selectionBox[KFlake::LeftMiddleHandle] = (outline.value(3)+outline.value(0))/2;
    m_selectionBox[KFlake::TopLeftHandle] = outline.value(0);
    if (koSelection()->count() == 1) {
#if 0        // TODO detect mirroring
        KShape *s = koSelection()->firstSelectedShape();

        if (s->scaleX() < 0) { // vertically mirrored: swap left / right
            qSwap(m_selectionBox[KFlake::TopLeftHandle], m_selectionBox[KFlake::TopRightHandle]);
            qSwap(m_selectionBox[KFlake::LeftMiddleHandle], m_selectionBox[KFlake::RightMiddleHandle]);
            qSwap(m_selectionBox[KFlake::BottomLeftHandle], m_selectionBox[KFlake::BottomRightHandle]);
        }
        if (s->scaleY() < 0) { // vertically mirrored: swap top / bottom
            qSwap(m_selectionBox[KFlake::TopLeftHandle], m_selectionBox[KFlake::BottomLeftHandle]);
            qSwap(m_selectionBox[KFlake::TopMiddleHandle], m_selectionBox[KFlake::BottomMiddleHandle]);
            qSwap(m_selectionBox[KFlake::TopRightHandle], m_selectionBox[KFlake::BottomRightHandle]);
        }
#endif
    }
}

void DefaultTool::activate(ToolActivation, const QSet<KShape*> &)
{
    m_mouseWasInsideHandles = false;
    m_lastHandle = KFlake::NoHandle;
    setCursor(Qt::ArrowCursor);
    repaintDecorations();
    delete m_guideLine;
    m_guideLine = new GuideLine();
    updateActions();
}

void DefaultTool::selectionAlignHorizontalLeft()
{
    selectionAlign(KShapeAlignCommand::HorizontalLeftAlignment);
}

void DefaultTool::selectionAlignHorizontalCenter()
{
    selectionAlign(KShapeAlignCommand::HorizontalCenterAlignment);
}

void DefaultTool::selectionAlignHorizontalRight()
{
    selectionAlign(KShapeAlignCommand::HorizontalRightAlignment);
}

void DefaultTool::selectionAlignVerticalTop()
{
    selectionAlign(KShapeAlignCommand::VerticalTopAlignment);
}

void DefaultTool::selectionAlignVerticalCenter()
{
    selectionAlign(KShapeAlignCommand::VerticalCenterAlignment);
}

void DefaultTool::selectionAlignVerticalBottom()
{
    selectionAlign(KShapeAlignCommand::VerticalBottomAlignment);
}

void DefaultTool::selectionGroup()
{
    KShapeSelection* selection = koSelection();
    if (! selection)
        return;

    QList<KShape*> selectedShapes = selection->selectedShapes(KFlake::TopLevelSelection);
    QList<KShape*> groupedShapes;

    // only group shapes with an unselected parent
    foreach (KShape* shape, selectedShapes) {
        if (! selectedShapes.contains(shape->parent()) && shape->isEditable()) {
            groupedShapes << shape;
        }
    }
    if (groupedShapes.count() < 2)
        return;
    canvas()->addCommand(KShapeGroupCommand::createCommand(groupedShapes, canvas()->shapeController()));
}

void DefaultTool::selectionUngroup()
{
    KShapeSelection* selection = canvas()->shapeManager()->selection();
    if (! selection)
        return;

    QList<KShape*> selectedShapes = selection->selectedShapes(KFlake::TopLevelSelection);
    QList<KShape*> containerSet;

    // only ungroup shape groups with an unselected parent
    foreach (KShape* shape, selectedShapes) {
        if (!selectedShapes.contains(shape->parent()) && shape->isEditable()) {
            containerSet << shape;
        }
    }

    QUndoCommand *cmd = 0;

    // add a ungroup command for each found shape container to the macro command
    foreach(KShape *shape, containerSet) {
        KShapeGroup *group = dynamic_cast<KShapeGroup*>(shape);
        if (group) {
            cmd = cmd ? cmd : new QUndoCommand(i18n("Ungroup shapes"));
            canvas()->shapeController()->removeShape(group, cmd); // removes parent and children.
            QList<KShape*> children = group->shapes();
            foreach (KShape *shape, children) {
                // re-add children to document correctly, so the doc can notice them as toplevels
                canvas()->shapeController()->addShape(shape, cmd);
            }
            new KShapeUngroupCommand(group, group->shapes(),
                    group->parent()? QList<KShape*>(): canvas()->shapeManager()->topLevelShapes(),
                    cmd);
        }
    }
    if (cmd) {
        canvas()->addCommand(cmd);
    }
}

void DefaultTool::selectionAlign(KShapeAlignCommand::Align align)
{
    KShapeSelection* selection = canvas()->shapeManager()->selection();
    if (! selection)
        return;

    QList<KShape*> selectedShapes = selection->selectedShapes(KFlake::TopLevelSelection);
    if (selectedShapes.count() < 1)
        return;

    QList<KShape*> editableShapes = filterEditableShapes(selectedShapes);

    // TODO add an option to the widget so that one can align to the page
    // with multiple selected shapes too

    QRectF bb;

    // single selected shape is automatically aligned to document rect
    if (editableShapes.count() == 1 ) {
        if (!canvas()->resourceManager()->hasResource(KCanvasResource::PageSize))
            return;
        bb = QRectF(QPointF(0,0), canvas()->resourceManager()->sizeResource(KCanvasResource::PageSize));
    } else {
        foreach( KShape * shape, editableShapes ) {
            bb |= shape->boundingRect();
        }
    }

    KShapeAlignCommand *cmd = new KShapeAlignCommand(editableShapes, align, bb);

    canvas()->addCommand(cmd);
    selection->updateSizeAndPosition();
}

void DefaultTool::selectionBringToFront()
{
    selectionReorder(KShapeReorderCommand::BringToFront);
}

void DefaultTool::selectionMoveUp()
{
    selectionReorder(KShapeReorderCommand::RaiseShape);
}

void DefaultTool::selectionMoveDown()
{
    selectionReorder(KShapeReorderCommand::LowerShape);
}

void DefaultTool::selectionSendToBack()
{
    selectionReorder(KShapeReorderCommand::SendToBack);
}

void DefaultTool::selectionReorder(KShapeReorderCommand::MoveShapeType order)
{
    KShapeSelection* selection = canvas()->shapeManager()->selection();
    if (! selection)
        return;

    QList<KShape*> selectedShapes = selection->selectedShapes(KFlake::TopLevelSelection);
    if (selectedShapes.count() < 1)
        return;

    QList<KShape*> editableShapes = filterEditableShapes(selectedShapes);
    if (editableShapes.count() < 1)
        return;

    QUndoCommand *cmd = KShapeReorderCommand::createCommand(editableShapes, canvas()->shapeManager(), order);
    canvas()->addCommand(cmd);
}

QMap<QString, QWidget *> DefaultTool::createOptionWidgets()
{
    QMap<QString, QWidget *> widgets;
    widgets.insert(i18n("Arrange"), new DefaultToolArrangeWidget(this));
    widgets.insert(i18n("Geometry"), new DefaultToolWidget(this));
    widgets.insert(i18n("Snapping"), canvas()->createSnapGuideConfigWidget());
    return widgets;
}

void DefaultTool::resourceChanged(int key, const QVariant & res)
{
    if (key == HotPosition) {
        m_hotPosition = static_cast<KFlake::Position>(res.toInt());
        repaintDecorations();
    }
}

KInteractionStrategy *DefaultTool::createStrategy(KPointerEvent *event)
{
    // reset the move by keys when a new strategy is created otherwise we might change the
    // command after a new command was added. This happend when you where faster than the timer.
    m_moveCommand = 0;

    KShapeManager *shapeManager = canvas()->shapeManager();
    KShapeSelection *select = shapeManager->selection();
    bool insideSelection;
    KFlake::SelectionHandle handle = handleAt(event->point, &insideSelection);

    bool editableShape = editableShapesCount(select->selectedShapes());

    if (event->buttons() & Qt::MidButton) {
        // change the hot selection position when middle clicking on a handle
        KFlake::Position newHotPosition = m_hotPosition;
        switch (handle) {
        case KFlake::TopLeftHandle:
            newHotPosition = KFlake::TopLeftCorner;
            break;
        case KFlake::TopRightHandle:
            newHotPosition = KFlake::TopRightCorner;
            break;
        case KFlake::BottomLeftHandle:
            newHotPosition = KFlake::BottomLeftCorner;
            break;
        case KFlake::BottomRightHandle:
            newHotPosition = KFlake::BottomRightCorner;
            break;
        default: {
            // check if we had hit the center point
            const KViewConverter * converter = canvas()->viewConverter();
            QPointF pt = converter->documentToView(event->point-select->absolutePosition());
            if (qAbs(pt.x()) < HANDLE_DISTANCE && qAbs(pt.y()) < HANDLE_DISTANCE)
                newHotPosition = KFlake::CenteredPosition;
            break;
        }
        }
        if (m_hotPosition != newHotPosition)
            canvas()->resourceManager()->setResource(HotPosition, newHotPosition);
        return 0;
    }

    bool selectMultiple = event->modifiers() & Qt::ControlModifier;
    bool selectNextInStack = event->modifiers() & Qt::ShiftModifier;

    if (editableShape) {
        // manipulation of selected shapes goes first
        if (handle != KFlake::NoHandle) {
            if (event->buttons() == Qt::LeftButton) {
                // resizing or shearing only with left mouse button
                if (insideSelection)
                    return new ShapeResizeStrategy(this, event->point, handle);
                if (handle == KFlake::TopMiddleHandle || handle == KFlake::RightMiddleHandle ||
                        handle == KFlake::BottomMiddleHandle || handle == KFlake::LeftMiddleHandle)
                    return new ShapeShearStrategy(this, event->point, handle);
            }
            // rotating is allowed for rigth mouse button too
            if (handle == KFlake::TopLeftHandle || handle == KFlake::TopRightHandle ||
                handle == KFlake::BottomLeftHandle || handle == KFlake::BottomRightHandle)
                return new ShapeRotateStrategy(this, event->point, event->buttons());
        }
        if (! (selectMultiple || selectNextInStack) && event->buttons() == Qt::LeftButton) {
            const QPainterPath outlinePath = select->transformation().map(select->outline());
            if (outlinePath.contains(event->point) || outlinePath.intersects(handlePaintRect(event->point)))
                return new ShapeMoveStrategy(this, event->point);
        }
    }

    if ((event->buttons() & Qt::LeftButton) == 0)
        return 0;  // Nothing to do for middle/right mouse button

    // check if we clicked near any of the selected connectors end-points.
    qreal clickDistance = HANDLE_DISTANCE/2;
    const KViewConverter *viewConverter = canvas()->viewConverter();
    if (viewConverter)
        clickDistance = viewConverter->viewToDocumentX(clickDistance);
    foreach (KShapeConnection *connection, m_selectedConnections) {
        QLineF distance1(event->point, connection->startPoint());
        if (distance1.length() < clickDistance)
            return new ConnectionChangeStrategy(this, connection, event->point,
                    ConnectionChangeStrategy::StartPointDrag);
        QLineF distance2(event->point, connection->endPoint());
        if (distance2.length() < clickDistance)
            return new ConnectionChangeStrategy(this, connection, event->point,
                    ConnectionChangeStrategy::EndPointDrag);
    }

    if (selectMultiple) {
        KShapeConnection *connection = shapeManager->connectionAt(event->point);
        if (connection && m_selectedConnections.contains(connection)) {
            repaintDecorations();
            m_selectedConnections.removeAll(connection);
            return 0;
        }
    }

    KShape *shape = shapeManager->shapeAt(event->point, selectNextInStack ? KFlake::NextUnselected : KFlake::ShapeOnTop);

    if (!shape && handle == KFlake::NoHandle) {
        KShapeConnection *connection = shapeManager->connectionAt(event->point);
        if (connection) { // clicked on a shape-to-shape connector
            if (!selectMultiple && (select->count() > 0 || !m_selectedConnections.isEmpty())) {
                repaintDecorations();
                select->deselectAll();
                m_selectedConnections.clear();
            }
            m_selectedConnections << connection;
            repaintDecorations();
            return 0;
        }

        // check if we have hit a guide
        if (m_guideLine->isValid()) {
            m_guideLine->select();
            return 0;
        }
        if (! selectMultiple) {
            repaintDecorations();
            select->deselectAll();
            m_selectedConnections.clear();
        }
        return new KShapeRubberSelectStrategy(this, event->point);
    }

    if (select->isSelected(shape)) {
        if (selectMultiple) {
            repaintDecorations();
            select->deselect(shape);
        }
    }
    else if (handle == KFlake::NoHandle) { // clicked on shape which is not selected
        repaintDecorations();
        if (!selectMultiple)
            shapeManager->selection()->deselectAll();
        select->select(shape, selectNextInStack ? KShapeSelection::NonRecursive : KShapeSelection::Recursive);
        repaintDecorations();
        ShapeMoveStrategy *newStrategy = new ShapeMoveStrategy(this, event->point);
        if (!selectMultiple) {
            QList<KShape*> myShape;
            myShape << shape;
            QString toolName = KToolManager::instance()->preferredToolForSelection(myShape);
            KToolFactoryBase *factory = KToolRegistry::instance()->value(toolName);
            if (factory) {
                const KToolFactoryBase::ShapeSelectionFlags flags = factory->autoActivateFlags();
                bool autoActivate = !shape->isContentProtected(); // Default don't use those
                if (flags & KToolFactoryBase::ContentProtected)
                    autoActivate = shape->isGeometryProtected();
                if ((flags & KToolFactoryBase::NoShapeMatch) == KToolFactoryBase::NoShapeMatch)
                    autoActivate = false;
                if (autoActivate && (flags & KToolFactoryBase::ShapeGeometryLocked))
                    autoActivate = shape->isGeometryProtected();
                if (autoActivate && (flags & KToolFactoryBase::ShapeGeometryUnLocked))
                    autoActivate = !shape->isGeometryProtected();
                if (autoActivate)
                    newStrategy->setAutoActivateOnComplete();
            }
        }
        return newStrategy;
    }
    return 0;
}

void DefaultTool::updateActions()
{
    KShapeSelection *selection(koSelection());
    if (!selection) {
        action("object_order_front")->setEnabled(false);
        action("object_order_raise")->setEnabled(false);
        action("object_order_lower")->setEnabled(false);
        action("object_order_back")->setEnabled(false);
        action("object_align_horizontal_left")->setEnabled(false);
        action("object_align_horizontal_center")->setEnabled(false);
        action("object_align_horizontal_right")->setEnabled(false);
        action("object_align_vertical_top")->setEnabled(false);
        action("object_align_vertical_center")->setEnabled(false);
        action("object_align_vertical_bottom")->setEnabled(false);
        action("object_group")->setEnabled(false);
        action("object_ungroup")->setEnabled(false);
        return;
    }

    QList<KShape*> editableShapes = filterEditableShapes(selection->selectedShapes(KFlake::TopLevelSelection));
    bool enable = editableShapes.count () > 0;
    action("object_order_front")->setEnabled(enable);
    action("object_order_raise")->setEnabled(enable);
    action("object_order_lower")->setEnabled(enable);
    action("object_order_back")->setEnabled(enable);
    enable = (editableShapes.count () > 1) || (enable && canvas()->resourceManager()->hasResource(KCanvasResource::PageSize));
    action("object_align_horizontal_left")->setEnabled(enable);
    action("object_align_horizontal_center")->setEnabled(enable);
    action("object_align_horizontal_right")->setEnabled(enable);
    action("object_align_vertical_top")->setEnabled(enable);
    action("object_align_vertical_center")->setEnabled(enable);
    action("object_align_vertical_bottom")->setEnabled(enable);

    action("object_group")->setEnabled(editableShapes.count() > 1);
    bool groupShape = false;
    foreach (KShape * shape, editableShapes) {
        if (dynamic_cast<KShapeGroup *>(shape)) {
            groupShape = true;
            break;
        }
    }
    action("object_ungroup")->setEnabled(groupShape);

    emit selectionChanged(selection->count());
}

KToolSelection* DefaultTool::selection()
{
    return m_selectionHandler;
}

QList<KShape*> DefaultTool::filterEditableShapes( const QList<KShape*> &shapes )
{
    QList<KShape*> editableShapes;
    foreach( KShape * shape, shapes ) {
        if (shape->isEditable())
            editableShapes.append(shape);
    }

    return editableShapes;
}

uint DefaultTool::editableShapesCount( const QList<KShape*> &shapes )
{
    uint count = 0;
    foreach( KShape * shape, shapes ) {
        if (shape->isEditable())
            count++;
    }

    return count;
}

#include <DefaultTool.moc>
