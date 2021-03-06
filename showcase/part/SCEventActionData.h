/* This file is part of the KDE project
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef SCEVENTACTIONDATA_H
#define SCEVENTACTIONDATA_H

#include "showcase_export.h"

class SCSoundCollection;
class KShape;
class KEventAction;
class SCSoundCollection;

class SHOWCASE_EXPORT SCEventActionData
{
public:
    SCEventActionData(KShape * shape, KEventAction * eventAction , SCSoundCollection * soundCollection);
    ~SCEventActionData();

    SCSoundCollection * soundCollection() const;

    /**
     * Get the shape
     *
     * @return the shape or 0 if there is none.
     */
    KShape *shape() const;

    /**
     * Get the event action
     *
     * @return the event action or 0 if there is none
     */
    KEventAction *eventAction() const;

private:
    class Private;
    Private * const d;
};

#endif /* SCEVENTACTIONDATA_H */
