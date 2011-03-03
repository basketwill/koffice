/* This file is part of the KDE project
   Copyright (C) 2008 Casper Boemann <cbr@boemann.dk>
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>

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
   Boston, MA 02110-1301, USA.
*/

#include "SCClickActionDocker.h"

#include <QVBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QEvent>
#include <klocale.h>

#include <KFileDialog>

#include <KoPACanvas.h>
#include <SCDocument.h>
#include <KoCanvasBase.h>
#include <KoSelection.h>
#include <KoShapeController.h>
#include <KoShapeManager.h>
#include <KoEventAction.h>
#include <KoEventActionFactoryBase.h>
#include <KoEventActionRegistry.h>
#include <SCEventActionWidget.h>
#include "SCSoundData.h"
#include <Showcase.h>
#include "SCSoundCollection.h"
#include "SCView.h"
#include "SCPage.h"
#include "SCEventActionData.h"

#include <kdebug.h>

SCClickActionDocker::SCClickActionDocker(QWidget* parent, Qt::WindowFlags flags)
: QWidget(parent, flags)
, m_view(0)
, m_soundCollection(0)
{
    setObjectName("SCClickActionDocker");
    // setup widget layout
    QVBoxLayout* layout = new QVBoxLayout;
    m_cbPlaySound = new QComboBox();
    //layout->addWidget(m_cbPlaySound);

    QList<KoEventActionFactoryBase *> factories = KoEventActionRegistry::instance()->presentationEventActions();
    foreach (KoEventActionFactoryBase * factory, factories) {
        QWidget * optionWidget = factory->createOptionWidget();
        layout->addWidget(optionWidget);
        m_eventActionWidgets.insert(factory->id(), optionWidget);
        connect(optionWidget, SIGNAL(addCommand(QUndoCommand *)),
                 this, SLOT(addCommand(QUndoCommand *)));
    }

    setLayout(layout);
}

void SCClickActionDocker::selectionChanged()
{
    if(! m_canvas)
        return;
    KoSelection *selection = m_canvas->shapeManager()->selection();
    KoShape *shape = selection->firstSelectedShape();

    if (shape) {
        QSet<KoEventAction *> eventActions = shape->eventActions();
        QMap<QString, KoEventAction*> eventActionMap;
        foreach (KoEventAction * eventAction, eventActions) {
            eventActionMap.insert(eventAction->id(), eventAction);
        }

        QMap<QString, QWidget *>::const_iterator it(m_eventActionWidgets.constBegin());

        for (; it != m_eventActionWidgets.constEnd(); ++it)  {
            SCEventActionWidget *actionWidget = dynamic_cast<SCEventActionWidget*>(it.value());
            if (actionWidget) {
                // if it is not in the map a default value 0 pointer will be returned
                SCEventActionData data(shape, eventActionMap.value(it.key()), m_soundCollection);
                actionWidget->setData(&data);
            }
        }
    }
    else {
        foreach (QWidget *widget, m_eventActionWidgets) {
            SCEventActionWidget *actionWidget = dynamic_cast<SCEventActionWidget*>(widget);
            if (actionWidget) {
                SCEventActionData data(0, 0, m_soundCollection);
                actionWidget->setData(&data);
            }
        }
    }
}

void SCClickActionDocker::setCanvas(KoCanvasBase *canvas)
{
    m_canvas = canvas;

    if (m_canvas) {
        connect(m_canvas->shapeManager(), SIGNAL(selectionChanged()),
                this, SLOT(selectionChanged()));
   }

    selectionChanged();
}

void SCClickActionDocker::setView(KoPAViewBase  *view)
{
    m_view = view;
    if (m_view->kopaDocument()->resourceManager()->hasResource(Showcase::SoundCollection)) {
        QVariant variant = m_view->kopaDocument()->resourceManager()->resource(Showcase::SoundCollection);
        m_soundCollection = variant.value<SCSoundCollection*>();
    }

    setCanvas(view->kopaCanvas());
}

void SCClickActionDocker::addCommand(QUndoCommand * command)
{
    if (m_view) {
        m_view->kopaCanvas()->addCommand(command);
    }
}

#include "SCClickActionDocker.moc"