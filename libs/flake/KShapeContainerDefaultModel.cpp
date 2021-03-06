/* This file is part of the KDE project
 * Copyright (C) 2006-2007, 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2009 Thorsten Zachmann <zachmann@kde.org>
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

#include "KShapeContainerDefaultModel.h"

#include "KShapeContainer.h"

class KShapeContainerDefaultModel::Private
{
public:
    class Relation
    {
    public:
        explicit Relation(KShape *child)
        : inside(false),
        inheritsTransform(false),
        m_child(child)
        {}

        KShape* child()
        {
            return m_child;
        }

        uint inside : 1; ///< if true, the child will be clipped by the parent.
        uint inheritsTransform : 1;

    private:
        KShape *m_child;
    };

    ~Private()
    {
        qDeleteAll(relations);
    }

    Relation* findRelation(const KShape *child) const
    {
        foreach (Relation *relation, relations) {
            if (relation->child() == child) {
                return relation;
            }
        }
        return 0;
    }


    // TODO use a QMap<KShape*, bool> instead this should speed things up a bit
    QList<Relation *> relations;
};

KShapeContainerDefaultModel::KShapeContainerDefaultModel()
: d(new Private())
{
}

KShapeContainerDefaultModel::~KShapeContainerDefaultModel()
{
    delete d;
}

void KShapeContainerDefaultModel::add(KShape *child)
{
    Private::Relation *r = new Private::Relation(child);
    d->relations.append(r);
}

void KShapeContainerDefaultModel::proposeMove(KShape *shape, QPointF &move)
{
    KShapeContainer *parent = shape->parent();
    bool allowedToMove = true;
    while (allowedToMove && parent) {
        allowedToMove = parent->isEditable();
        parent = parent->parent();
    }
    if (! allowedToMove) {
        move.setX(0);
        move.setY(0);
    }
}


void KShapeContainerDefaultModel::setClipped(const KShape *child, bool clipping)
{
    Private::Relation *relation = d->findRelation(child);
    if (relation == 0)
        return;
    if (relation->inside == clipping)
        return;
    relation->child()->update(); // mark old canvas-location as in need of repaint (aggregated)
    relation->inside = clipping;
    relation->child()->updateGeometry();
    relation->child()->update(); // mark new area as in need of repaint
}

bool KShapeContainerDefaultModel::isClipped(const KShape *child) const
{
    Private::Relation *relation = d->findRelation(child);
    return relation ? relation->inside: false;
}

void KShapeContainerDefaultModel::remove(KShape *child)
{
    Private::Relation *relation = d->findRelation(child);
    if (relation == 0)
        return;
    d->relations.removeAll(relation);
}

int KShapeContainerDefaultModel::count() const
{
    return d->relations.count();
}

QList<KShape*> KShapeContainerDefaultModel::shapes() const
{
    QList<KShape*> answer;
    foreach(Private::Relation *relation, d->relations) {
        answer.append(relation->child());
    }
    return answer;
}

bool KShapeContainerDefaultModel::isChildLocked(const KShape *child) const
{
    return child->isGeometryProtected();
}

void KShapeContainerDefaultModel::containerChanged(KShapeContainer *, KShape::ChangeType)
{
}

void KShapeContainerDefaultModel::setInheritsTransform(const KShape *shape, bool inherit)
{
    Private::Relation *relation = d->findRelation(shape);
    if (relation == 0)
        return;
    if (relation->inheritsTransform == inherit)
        return;
    relation->child()->update(); // mark old canvas-location as in need of repaint (aggregated)
    relation->inheritsTransform = inherit;
    relation->child()->updateGeometry();
    relation->child()->update(); // mark new area as in need of repaint
}

bool KShapeContainerDefaultModel::inheritsTransform(const KShape *shape) const
{
    Private::Relation *relation = d->findRelation(shape);
    return relation ? relation->inheritsTransform: false;
}

