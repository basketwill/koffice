/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
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
#include "KoToolProxy.h"

#include "KoTool.h"
#include "KoPointerEvent.h"
#include "KoInputDevice.h"
#include "KoToolManager.h"
#include "KoToolSelection.h"
#include "KoCanvasBase.h"
#include "KoCanvasController.h"
#include "KoShapeManager.h"

#include <kdebug.h>
#include <QTimer>

/* Unused for now.
#if 0
namespace
{

    // Helper class to determine when the user might switch between
    // tablet and mouse.
    class TabletProximityFilter {
    public:

        TabletProximityFilter( KoToolManager * manager )
            : m_manager( manager )
            {
            }

        virtual ~TabletProximityFilter();

        bool eventFilter( QObject * object,  QEvent * event ) {

            if ( object == qApp ) {
                switch( event->type() ) {
                    case QEvent::TabletEnterProximity:
                        break;
                    case QEvent::TabletLeaveProximity:
                        break;
                    default:
                        break;
                }
            }
            return false;
        }

    private:

        KoToolManager * m_manager;
    };

}
#endif */


class KoToolProxy::Private
{
public:
    Private(KoToolProxy *p) : activeTool(0), tabletPressed(false), hasSelection(false), controller(0), parent(p) {
        scrollTimer.setInterval(100);
        mouseLeaveWorkaround = false;
    }

    void timeout() { // Auto scroll the canvas
        Q_ASSERT(controller);
        int offsetX = controller->canvasOffsetX();
        int offsetY = controller->canvasOffsetY();
        // get the points version of 10 pixels offset.
        QPointF offset = controller->canvas()->viewConverter()->viewToDocument(QPointF(10, 10));
        QRectF mouseArea(scrollEdgePoint, QSizeF(offset.x(), offset.y()));
        mouseArea.setTopLeft(mouseArea.center());

        controller->ensureVisible(mouseArea, true);

        QPoint moved(offsetX - controller->canvasOffsetX(), offsetY - controller->canvasOffsetY());
        if (moved.x() == 0 && moved.y() == 0)
            return;
        scrollEdgePoint += controller->canvas()->viewConverter()->viewToDocument(moved);

        QMouseEvent event(QEvent::MouseMove, scrollEdgePoint.toPoint(), Qt::LeftButton, Qt::LeftButton, 0);
        KoPointerEvent ev(&event, scrollEdgePoint);
        activeTool->mouseMoveEvent(&ev);
    }

    void checkAutoScroll(const KoPointerEvent &event) {
        if (controller == 0) return;
        if (!activeTool->wantsAutoScroll()) return;
        if (!event.isAccepted()) return;
        if (event.buttons() != Qt::LeftButton) return;
        scrollEdgePoint = event.point;
        if (! scrollTimer.isActive())
            scrollTimer.start();
    }

    void selectionChanged(bool newSelection) {
        if (hasSelection == newSelection)
            return;
        hasSelection = newSelection;
        emit parent->selectionChanged(hasSelection);
    }

    KoTool *activeTool;
    bool tabletPressed;
    bool hasSelection;
    QTimer scrollTimer;
    QPointF scrollEdgePoint;
    KoCanvasController *controller;
    KoToolProxy *parent;

    bool mouseLeaveWorkaround; // up until at least 4.3.0 we get a mouse move event when the tablet leaves the canvas.
};

KoToolProxy::KoToolProxy(KoCanvasBase *canvas, QObject *parent)
        : QObject(parent),
        d(new Private(this))
{
    KoToolManager::instance()->registerToolProxy(this, canvas);

    connect(&d->scrollTimer, SIGNAL(timeout()), this, SLOT(timeout()));
}

KoToolProxy::~KoToolProxy()
{
    delete d;
}

void KoToolProxy::paint(QPainter &painter, const KoViewConverter &converter)
{
    if (d->activeTool) d->activeTool->paint(painter, converter);
}

void KoToolProxy::repaintDecorations()
{
    if (d->activeTool) d->activeTool->repaintDecorations();
}

void KoToolProxy::tabletEvent(QTabletEvent *event, const QPointF &point)
{
    KoInputDevice id(event->device(), event->pointerType(), event->uniqueId());
    KoToolManager::instance()->switchInputDevice(id);

    KoPointerEvent ev(event, point);
    switch (event->type()) {
    case QEvent::TabletPress:
        ev.setTabletButton(Qt::LeftButton);
        if (! d->tabletPressed && d->activeTool)
            d->activeTool->mousePressEvent(&ev);
        d->tabletPressed = true;
        break;
    case QEvent::TabletRelease:
        ev.setTabletButton(Qt::LeftButton);
        d->tabletPressed = false;
        d->scrollTimer.stop();
        if (d->activeTool) d->activeTool->mouseReleaseEvent(&ev);
        break;
    case QEvent::TabletMove:
        if (d->tabletPressed) ev.setTabletButton(Qt::LeftButton);
        if (d->activeTool) d->activeTool->mouseMoveEvent(&ev);
        d->checkAutoScroll(ev);
    default:
        ; // ignore the rest.
    }

    // Always accept tablet events as they are useless to parent widgets and they will
    // get re-send as mouseevents if we don't accept them.
    event->accept();
    d->mouseLeaveWorkaround = true;
}

void KoToolProxy::mousePressEvent(QMouseEvent *event, const QPointF &point)
{
    d->mouseLeaveWorkaround = false;
    KoInputDevice id;
    KoToolManager::instance()->switchInputDevice(id);

    if (d->tabletPressed) // refuse to send a press unless there was a release first.
        return;

    KoPointerEvent ev(event, point);
    if (d->activeTool) d->activeTool->mousePressEvent(&ev);
}

void KoToolProxy::mouseDoubleClickEvent(QMouseEvent *event, const QPointF &point)
{
    d->mouseLeaveWorkaround = false;
    KoInputDevice id;
    KoToolManager::instance()->switchInputDevice(id);
    if (d->activeTool == 0)  return;

    KoPointerEvent ev(event, point);
    d->activeTool->mouseDoubleClickEvent(&ev);
    if (! event->isAccepted())
        d->activeTool->m_canvas->shapeManager()->suggestChangeTool(&ev);
}

void KoToolProxy::mouseMoveEvent(QMouseEvent *event, const QPointF &point)
{
    if (d->mouseLeaveWorkaround) {
        d->mouseLeaveWorkaround = false;
        return;
    }
    KoInputDevice id;
    KoToolManager::instance()->switchInputDevice(id);
    if (d->activeTool == 0)
        return;

    KoPointerEvent ev(event, point);
    d->activeTool->mouseMoveEvent(&ev);

    d->checkAutoScroll(ev);
}

void KoToolProxy::mouseReleaseEvent(QMouseEvent *event, const QPointF &point)
{
    d->mouseLeaveWorkaround = false;
    KoInputDevice id;
    KoToolManager::instance()->switchInputDevice(id);
    d->scrollTimer.stop();

    KoPointerEvent ev(event, point);
    if (d->activeTool) d->activeTool->mouseReleaseEvent(&ev);
}

void KoToolProxy::keyPressEvent(QKeyEvent *event)
{
    if (d->activeTool) d->activeTool->keyPressEvent(event);
}

void KoToolProxy::keyReleaseEvent(QKeyEvent *event)
{
    if (d->activeTool) d->activeTool->keyReleaseEvent(event);
}

void KoToolProxy::wheelEvent(QWheelEvent * event, const QPointF &point)
{
    KoPointerEvent ev(event, point);
    if (d->activeTool) d->activeTool->wheelEvent(&ev);
}

QVariant KoToolProxy::inputMethodQuery(Qt::InputMethodQuery query, const KoViewConverter &converter) const
{
    if (d->activeTool)
        return d->activeTool->inputMethodQuery(query, converter);
    return QVariant();
}

void KoToolProxy::inputMethodEvent(QInputMethodEvent *event)
{
    if (d->activeTool) d->activeTool->inputMethodEvent(event);
}

KoToolSelection* KoToolProxy::selection()
{
    if (d->activeTool)
        return d->activeTool->selection();
    return 0;
}

void KoToolProxy::setActiveTool(KoTool *tool)
{
    Q_ASSERT(tool);
    if (d->activeTool)
        disconnect(d->activeTool, SIGNAL(selectionChanged(bool)), this, SLOT(selectionChanged(bool)));
    d->activeTool = tool;
    connect(d->activeTool, SIGNAL(selectionChanged(bool)), this, SLOT(selectionChanged(bool)));
    d->selectionChanged(d->activeTool->selection() && d->activeTool->selection()->hasSelection());
    emit toolChanged(tool->toolId());
}

void KoToolProxy::setCanvasController(KoCanvasController *controller)
{
    d->controller = controller;
}

QHash<QString, QAction*> KoToolProxy::actions() const
{
    return d->activeTool ? d->activeTool->actions() : QHash<QString, QAction*>();
}

void KoToolProxy::cut()
{
    if (d->activeTool)
        d->activeTool->cut();
}
void KoToolProxy::copy() const
{
    if (d->activeTool)
        d->activeTool->copy();
}

bool KoToolProxy::paste()
{
    if (d->activeTool)
        return d->activeTool->paste();
    return false;
}

QStringList KoToolProxy::supportedPasteMimeTypes() const
{
    if (d->activeTool)
        return d->activeTool->supportedPasteMimeTypes();

    return QStringList();
}

QList<QAction*> KoToolProxy::popupActionList() const
{
    if (d->activeTool)
        return d->activeTool->popupActionList();
    return QList<QAction*>();
}

void KoToolProxy::deleteSelection()
{
    if (d->activeTool)
        return d->activeTool->deleteSelection();
}

#include <KoToolProxy.moc>
