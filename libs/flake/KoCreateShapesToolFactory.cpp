/* This file is part of the KDE project
 *
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#include "KoCreateShapesToolFactory.h"
#include "KoCreateShapesTool.h"
#include "KoShapeControllerInterface.h"
#include "KoRectangleShape.h"

#include <klocale.h>

#include <QColor>
#include <QRectF>

KoCreateShapesToolFactory::KoCreateShapesToolFactory() {
}

KoCreateShapesToolFactory::~KoCreateShapesToolFactory() {
}

KoTool* KoCreateShapesToolFactory::createTool(KoCanvasBase *canvas) {
    class DummyShapeController : public KoShapeControllerInterface {
        public:
            DummyShapeController() {};
            ~DummyShapeController() {};
            void addShape(KoShape* shape) {};
            void removeShape(KoShape* shape) {};
            KoShape* createShape(const QRectF &outline) const {
                KoShape *rect = new KoRectangleShape();
                rect->setBackground(QColor(Qt::blue));
                rect->setPosition(outline.topLeft());
                rect->resize(outline.size());
                return rect;
            }
    };
    return new KoCreateShapesTool(canvas, new DummyShapeController());
}

KoID KoCreateShapesToolFactory::id() {
    return KoID("Create Shapes", i18n("Create Shapes"));
}

quint32 KoCreateShapesToolFactory::priority() const {
    return 1;
}

QString KoCreateShapesToolFactory::toolType() const {
    return QString("main");
}

QString KoCreateShapesToolFactory::tooltipText() const {
    return i18n("Create tooltip");
}

KoID* KoCreateShapesToolFactory::activationShapeId() const {
    return 0;
}

