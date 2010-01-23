/* This file is part of the KDE project
   Copyright (C) 2006 Rob Buis <buis@kde.org>
   Copyright (C) 2006 Thomas Zander <zander@kde.org>

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

#ifndef KOPATHSHAPEFACTORY_H
#define KOPATHSHAPEFACTORY_H

#include "KoShape.h"
#include "KoShapeFactory.h"

#include "KoXmlReader.h"

/// Factory for path shapes.
class FLAKE_TEST_EXPORT KoPathShapeFactory : public KoShapeFactory
{
public:
    /// constructor
    KoPathShapeFactory(QObject *parent, const QStringList&);
    ~KoPathShapeFactory() {}
    virtual KoShape *createDefaultShape(const QMap<QString, KoDataCenter *>  &dataCenterMap, KoResourceManager *documentResources = 0) const;
    bool supports(const KoXmlElement &element) const;
    /// reimplemented
    virtual void populateDataCenterMap(QMap<QString, KoDataCenter *>   &dataCenterMap);
    /// reimplemented
    virtual void newDocumentResourceManager(KoResourceManager *manager);
};

#endif
