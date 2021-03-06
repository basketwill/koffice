/* This file is part of the KDE project
 * Copyright (C) 2006-2011 Thomas Zander <zander@kde.org>
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

#ifndef KSHAPEMANAGER_P_H
#define KSHAPEMANAGER_P_H

#include "KShape_p.h"
#include "KShapeGroup.h"
#include <KRTree.h>

class KShapeManagerPrivate
{
public:
    KShapeManagerPrivate(KShapeManager *shapeManager, KCanvasBase *c);
    ~KShapeManagerPrivate();

    /**
     * Update the tree when there are shapes in m_aggregate4update. This is done so not all
     * updates to the tree are done when they are asked for but when they are needed.
     */
    void updateTree();

    /**
     * Recursively paints the given group shape to the specified painter
     * This is needed for filter effects on group shapes where the filter effect
     * applies to all the children of the group shape at once
     */
    void paintGroup(KShapeGroup *group, QPainter &painter, const KViewConverter &converter, bool forPrint);

    /**
     * Add a shape connection to the manager so it can be taken into account for drawing purposes.
     * Note that this is typically called by the shape instance only.
     */
    void addShapeConnection(KShapeConnection *connection);

    /**
     * Request a repaint to be queued.
     * The repaint will be restricted to the parameters rectangle, which is expected to be
     * in points (the document coordinates system of KShape) and it is expected to be
     * normalized and based in the global coordinates, not any local coordinates.
     * <p>This method will return immediately and only request a repaint. Successive calls
     * will be merged into an appropriate repaint action.
     * @param rect the rectangle (in pt) to queue for repaint.
     * @param shape the shape that is going to be redrawn; only needed when selectionHandles=true
     * @param selectionHandles if true; find out if the shape is selected and repaint its
     *   selection handles at the same time.
     */
    void update(const QRectF &rect, const KShape *shape = 0, bool selectionHandles = false);

    /**
     * Update the tree for finding the shapes.
     * This will remove the shape from the tree and will reinsert it again.
     * The update to the tree will be posponed until it is needed so that successive calls
     * will be merged into one.
     * @param shape the shape to updated its position in the tree.
     */
    void shapeGeometryChanged(KShape *shape);

    void shapeChanged(KShape *shape, KShape::ChangeType type);

    /**
     * Switch to editing the shape that is at the position of the event.
     * This method will check select a shape at the event position and switch to the default tool
     * for that shape, or switch to the default tool if there is no shape at the position.
     * @param event the event that holds the point where to look for a shape.
     */
    void suggestChangeTool(KPointerEvent *event);


    QPolygonF routeConnection(const QPointF &from, const QPointF &to);

    class DetectCollision
    {
    public:
        DetectCollision() {}
        void detect(KRTree<KShape *> &tree, KShape *s, int prevZIndex) {
            foreach(KShape *shape, tree.intersects(s->boundingRect())) {
                bool isChild = false;
                KShapeContainer *parent = s->parent();
                while (parent && !isChild) {
                    if (parent == shape)
                        isChild = true;
                    parent = parent->parent();
                }
                if (isChild)
                    continue;
                if (s->zIndex() <= shape->zIndex() && prevZIndex <= shape->zIndex())
                    // Moving a shape will only make it collide with shapes below it.
                    continue;
                if (shape->collisionDetection() && !shapesWithCollisionDetection.contains(shape))
                    shapesWithCollisionDetection.append(shape);
            }
        }

        void fireSignals() {
            foreach(KShape *shape, shapesWithCollisionDetection)
                shape->priv()->shapeChanged(KShape::CollisionDetected);
        }

    private:
        QList<KShape*> shapesWithCollisionDetection;
    };

    QList<KShape *> shapes;
    KShapeSelection *selection;
    KCanvasBase *canvas;
    KRTree<KShape *> tree;
    KRTree<KShapeConnection *> connectionTree;

    QSet<KShape *> aggregate4update;
    QHash<KShape*, int> shapeIndexesBeforeUpdate;
    KShapeManagerPaintingStrategy *strategy;
    KShapeManager *q;
};

#endif //KSHAPEMANAGER_P_H
