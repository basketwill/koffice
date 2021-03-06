/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#include "InsertVariableAction_p.h"
#include "KVariable.h"
#include "KInlineObjectFactoryBase.h"

#include <KCanvasBase.h>
#include <kpagedialog.h>

#include <KLocale>
#include <QLayout>
#include <QPointer>

InsertVariableAction::InsertVariableAction(KCanvasBase *base, KInlineObjectFactoryBase *factory, const KoInlineObjectTemplate &templ)
        : InsertInlineObjectActionBase(base, templ.name)
        , m_factory(factory)
        , m_templateId(templ.id)
        , m_properties(templ.properties)
        , m_templateName(templ.name)
{
}

KInlineObject *InsertVariableAction::createInlineObject()
{
    KInlineObject *io = m_factory->createInlineObject(m_properties);
    KVariable *variable = dynamic_cast<KVariable*>(io);
    Q_ASSERT(variable);
    QWidget *widget = variable->createOptionsWidget();
    if (widget) {
        if (widget->layout()) {
            widget->layout()->setMargin(0);
        }
        QPointer<KPageDialog> dialog = new KPageDialog(m_canvas->canvasWidget());
        dialog->setCaption(i18n("%1 Options", m_templateName));
        dialog->addPage(widget, QString());
        if (dialog->exec() != KPageDialog::Accepted) {
            delete variable;
            variable = 0;
        }
        delete dialog;
    }
    return variable;
}
