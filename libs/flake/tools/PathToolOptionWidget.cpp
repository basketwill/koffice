/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#include "PathToolOptionWidget_p.h"
#include "KoPathTool_p.h"
#include <KAction>

PathToolOptionWidget::PathToolOptionWidget(KoPathTool *tool, QWidget *parent)
        : QWidget(parent),
        m_tool(tool),
        m_shapePropertiesWidget(0)
{
    widget.setupUi(this);
    widget.corner->setDefaultAction(tool->action("pathpoint-corner"));
    widget.smooth->setDefaultAction(tool->action("pathpoint-smooth"));
    widget.symmetric->setDefaultAction(tool->action("pathpoint-symmetric"));
    widget.lineSegment->setDefaultAction(tool->action("pathsegment-line"));
    widget.curveSegment->setDefaultAction(tool->action("pathsegment-curve"));
    widget.linePoint->setDefaultAction(tool->action("pathpoint-line"));
    widget.curvePoint->setDefaultAction(tool->action("pathpoint-curve"));
    widget.addPoint->setDefaultAction(tool->action("pathpoint-insert"));
    widget.removePoint->setDefaultAction(tool->action("pathpoint-remove"));
    widget.breakPoint->setDefaultAction(tool->action("path-break-point"));
    widget.breakSegment->setDefaultAction(tool->action("path-break-segment"));
    widget.joinSegment->setDefaultAction(tool->action("pathpoint-join"));
    widget.mergePoints->setDefaultAction(tool->action("pathpoint-merge"));

    connect(widget.convertToPath, SIGNAL(released()), tool->action("convert-to-path"), SLOT(trigger()));
    widget.propertiesBox->setVisible(false);
}

PathToolOptionWidget::~PathToolOptionWidget()
{
    if (m_shapePropertiesWidget)
        m_shapePropertiesWidget->setParent(0); // disconnect. Its not ours to delete.
}

void PathToolOptionWidget::setSelectionType(Type type)
{
    if (type == PlainType)
        widget.stackedWidget->setCurrentIndex(0);
    else
        widget.stackedWidget->setCurrentIndex(1);
}

void PathToolOptionWidget::setShapePropertiesWidget(QWidget *propWidget)
{
    if (m_shapePropertiesWidget) {
        m_shapePropertiesWidget->setParent(0); // disconnect. Its not ours.
    }
    if (widget.propertiesBox->layout() == 0) {
        /*QHBoxLayout *hbox =*/ new QHBoxLayout(widget.propertiesBox);
        //widget.propertiesBox->setLayout(hbox); // TODO
    }
    if (propWidget) {
        widget.propertiesBox->layout()->addWidget(propWidget);
        widget.propertiesBox->setVisible(true);
    } else {
        widget.propertiesBox->setVisible(false);
    }
    m_shapePropertiesWidget = propWidget;
}

#include <PathToolOptionWidget_p.moc>
