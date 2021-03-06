/* This file is part of the KDE project

   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006-2007, 2010 Thomas Zander <zander@kde.org>

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

#ifndef KINTERACTIONTOOL_H
#define KINTERACTIONTOOL_H

#include "KToolBase.h"
#include "flake_export.h"

class KInteractionStrategy;
class KInteractionToolPrivate;

#define KoInteractionTool_ID "InteractionTool"

/**
 * The interaction tool adds to the normal KToolBase class the concept of strategies
 * as a means to get one tool to have different actions the user can perform using the mouse.
 * Each time the user presses the mouse until she releases the mouse a strategy object
 * will be created, used and disgarded.
 * If the usage of a tool fits this pattern you need to inherit from this class instead of the
 * plain KToolBase and reimplement your createStrategy() method which returns a tool-specific
 * strategy where all the real interaction code is placed.
 * A tool can then become as simple as this;
 * @code
    class MyTool : public KInteractionTool
    {
    public:
        MyTool::MyTool(KCanvasBase *canvas) : KInteractionTool(canvas) { }

        KInteractionStrategy *MyTool::createStrategy(KPointerEvent *event) {
            return new MyStrategy(this, m_canvas, event->point);
        }
    };
 * @endcode
 * Whereas your strategy (MyStrategy in the example) will contain the interaction code.
 */
class FLAKE_EXPORT KInteractionTool : public KToolBase
{
    Q_OBJECT
public:
    /**
     * Constructor for basic interaction tool where user actions are translated
     * and handled by interaction strategies of type KInteractionStrategy.
     * @param canvas the canvas this tool will be working for.
     */
    explicit KInteractionTool(KCanvasBase *canvas);
    virtual ~KInteractionTool();

public:
    virtual void paint(QPainter &painter, const KViewConverter &converter);

protected:
    virtual void mousePressEvent(KPointerEvent *event);
    virtual void mouseMoveEvent(KPointerEvent *event);
    virtual void mouseReleaseEvent(KPointerEvent *event);

    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);

    /// \internal
    KInteractionTool(KInteractionToolPrivate &dd);

    KInteractionStrategy *currentStrategy(); ///< the strategy that is 'in progress'
    /// Cancels the current strategy and deletes it.
    void cancelCurrentStrategy();

    /**
     * Reimplement this factory method to create your strategy to be used for mouse interaction.
     * @returns a new strategy, or 0 when there is nothing to do.
     */
    virtual KInteractionStrategy *createStrategy(KPointerEvent *event) = 0;

private:
    KInteractionTool(const KInteractionTool&);
    KInteractionTool& operator=(const KInteractionTool&);

    Q_DECLARE_PRIVATE(KInteractionTool)
};

#endif /* KOINTERACTIONTOOL_H */
