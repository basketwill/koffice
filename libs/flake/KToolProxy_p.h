/* This file is part of the KDE project
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
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

#ifndef KTOOLPROXYPRIVATE_P
#define KTOOLPROXYPRIVATE_P

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Flake API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//


#include <QTimer>
#include <QPointF>

class KPointerEvent;
class KToolBase;
class KCanvasController;
class KToolProxy;

class KToolProxyPrivate
{
public:
    KToolProxyPrivate(KToolProxy *p);

    void timeout(); // Auto scroll the canvas

    void checkAutoScroll(const KPointerEvent &event);

    void selectionChanged(bool newSelection);

    bool isActiveLayerEditable();

    /// the toolManager tells us which KCanvasController this toolProxy is working for.
    void setCanvasController(KCanvasController *controller);

    KToolBase *activeTool;
    bool tabletPressed;
    bool hasSelection;
    QTimer scrollTimer;
    QPoint scrollEdgePoint;
    KCanvasController *controller;
    KToolProxy *parent;

    // used to determine if the mouse-release is after a drag or a simple click
    QPoint mouseDownPoint;

    // up until at least 4.3.0 we get a mouse move event when the tablet leaves the canvas.
    bool mouseLeaveWorkaround;
};

#endif
