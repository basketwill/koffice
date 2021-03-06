/* This file is part of the KDE project
 * Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
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
#ifndef KSHAPEBORDERFACTORY_H
#define KSHAPEBORDERFACTORY_H

#include "flake_export.h"
#include <QObject>

class KShapeBorderBase;
class KShape;

class FLAKE_EXPORT KShapeBorderFactoryBase : public QObject
{
    Q_OBJECT
public:
    KShapeBorderFactoryBase(QObject *parent, const QString &id);
    virtual ~KShapeBorderFactoryBase();

    /**
     * @param targetShape the shape the border is for
     * @return a new border
     */
    virtual KShapeBorderBase *createBorder(KShape *targetShape) const = 0;

    /**
     * return the id for the border this factory creates.
     * @return the id for the border this factory creates.
     */
    QString id() const;

    /// return true if the stroke properties are configurable
    bool penStrokeConfigurable() const;

protected:
    /**
     * Set wheater the pen stroke is relevant for our border.
     * If true then this signifies that the border we create will use the stroke options (like line
     * type, line-width) that come from the pen of the border.
     * Notice that the color/pattern part of the pen is always configurable.
     * If the border does not respond to those changes, this should be set to false.
     * @param on the new state
     * @see KShapeBorderBase::pen()
     */
    void setPenStrokeConfigurable(bool on);

private:
    class Private;
    Private * const d;
};

#endif
