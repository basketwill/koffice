/* This file is part of the KDE project

   Copyright (C) 2006-2008 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006-2008 Thomas Zander <zander@kde.org>

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

#ifndef DEFAULTTOOL_H
#define DEFAULTTOOL_H

#include <KInteractionTool.h>
#include <KFlake.h>
#include <commands/KShapeAlignCommand.h>
#include <commands/KShapeReorderCommand.h>

#include <QPolygonF>
#include <QTime>

class KInteractionStrategy;
class KShapeMoveCommand;
class KShapeSelection;
class KShapeConnection;

/**
 * The default tool (associated with the arrow icon) implements the default
 * interactions you have with flake objects.<br>
 * The tool provides scaling, moving, selecting, rotation and soon skewing of
 * any number of shapes.
 * <p>Note that the implementation of those different strategies are delegated
 * to the InteractionStrategy class and its subclasses.
 */
class DefaultTool : public KInteractionTool
{
    Q_OBJECT
public:
    /**
     * Constructor for basic interaction tool where user actions are translated
     * and handled by interaction strategies of type KInteractionStrategy.
     * @param canvas the canvas this tool will be working for.
     */
    explicit DefaultTool(KCanvasBase *canvas);
    virtual ~DefaultTool();

    enum CanvasResource {
        HotPosition = 1410100299
    };

public:
    ///reimplemented
    virtual void paint(QPainter &painter, const KViewConverter &converter);

    ///reimplemented
    virtual void repaintDecorations();

    ///reimplemented
    virtual void copy() const;

    ///reimplemented
    virtual void deleteSelection();

    ///reimplemented
    virtual bool paste();
    ///reimplemented
    virtual KToolSelection* selection();

    /**
     * Returns which selection handle is at params point (or NoHandle if none).
     * @return which selection handle is at params point (or NoHandle if none).
     * @param point the location (in pt) where we should look for a handle
     * @param innerHandleMeaning this boolean is altered to true if the point
     *   is inside the selection rectangle and false if it is just outside.
     *   The value of innerHandleMeaning is undefined if the handle location is NoHandle
     */
    KFlake::SelectionHandle handleAt(const QPointF &point, bool *innerHandleMeaning = 0);

public slots:
    virtual void activate(ToolActivation toolActivation, const QSet<KShape*> &shapes);

private slots:
    void selectionAlignHorizontalLeft();
    void selectionAlignHorizontalCenter();
    void selectionAlignHorizontalRight();
    void selectionAlignVerticalTop();
    void selectionAlignVerticalCenter();
    void selectionAlignVerticalBottom();

    void selectionBringToFront();
    void selectionSendToBack();
    void selectionMoveUp();
    void selectionMoveDown();

    void selectionGroup();
    void selectionUngroup();

    /// Update actions on selection change
    void updateActions();

protected: // Events

    virtual void mousePressEvent(KPointerEvent *event);
    virtual void mouseMoveEvent(KPointerEvent *event);
    virtual void mouseReleaseEvent(KPointerEvent *event);
    virtual void mouseDoubleClickEvent(KPointerEvent *event);

    virtual void keyPressEvent(QKeyEvent *event);

    QMap<QString, QWidget *> createOptionWidgets();

    virtual KInteractionStrategy *createStrategy(KPointerEvent *event);

private:
    void setupActions();
    void recalcSelectionBox();
    void updateCursor();
    /// Returns rotation angle of given handle of the current selection
    qreal rotationOfHandle(KFlake::SelectionHandle handle, bool useEdgeRotation);

    void selectionAlign(KShapeAlignCommand::Align align);
    void selectionReorder(KShapeReorderCommand::MoveShapeType order);
    bool moveSelection(int direction, Qt::KeyboardModifiers modifiers);

    /// Returns selection rectangle adjusted by handle proximity threshold
    QRectF handlesSize();

    // convenience method;
    KShapeSelection * koSelection();

    void resourceChanged(int key, const QVariant & res);

    /// selects guide line at given position
    void selectGuideAtPosition(const QPointF &position);

    /// Returns list of editable shapes from the given list of shapes
    QList<KShape*> filterEditableShapes(const QList<KShape*> &shapes);

    /// Returns the number of editable shapes from the given list of shapes
    uint editableShapesCount(const QList<KShape*> &shapes);

    KFlake::SelectionHandle m_lastHandle;
    KFlake::Position m_hotPosition;
    bool m_mouseWasInsideHandles;
    QPointF m_selectionBox[8];
    QPolygonF m_selectionOutline;
    QPointF m_lastPoint;
    KShapeMoveCommand *m_moveCommand;
    QTime m_lastUsedMoveCommand;

    // TODO alter these 3 arrays to be static const instead
    QCursor m_sizeCursors[8];
    QCursor m_rotateCursors[8];
    QCursor m_shearCursors[8];
    qreal m_angle;
    KToolSelection *m_selectionHandler;
    friend class SelectionHandler;

    class GuideLine;
    GuideLine * m_guideLine;
    QList<KShapeConnection*> m_selectedConnections;
};

#endif
