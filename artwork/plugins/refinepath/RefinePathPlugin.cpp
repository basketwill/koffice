/* This file is part of the KDE project
   Copyright (C) 2002-2003,2005 Rob Buis <buis@kde.org>
   Copyright (C) 2003,2006 Laurent Montel <montel@kde.org>
   Copyright (C) 2003 Dirk Mueller <mueller@kde.org>
   Copyright (C) 2003 Stephan Binner <binner@kde.org>
   Copyright (C) 2003 Tomislav Lukman <tomislav.lukman@ck.t-com.hr>
   Copyright (C) 2004 Nicolas Goutte <nicolasg@snafu.de>
   Copyright (C) 2005,2007 Thomas Zander <zander@kde.org>
   Copyright (C) 2006 Tim Beaulen <tbscope@gmail.com>
   Copyright (C) 2006-2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2006 Stephan Kulow <coolo@kde.org>

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


#include "RefinePathPlugin.h"
#include "ArtworkPathRefineCommand.h"

#include <KToolManager.h>
#include <KCanvasController.h>
#include <KCanvasBase.h>
#include <KShapeManager.h>
#include <KShapeSelection.h>
#include <KPathShape.h>
#include <KParameterShape.h>

#include <kpluginfactory.h>
#include <kicon.h>
#include <knuminput.h>
#include <kactioncollection.h>
#include <kdebug.h>
#include <klocale.h>

#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QHBoxLayout>

K_PLUGIN_FACTORY(RefinePathPluginFactory, registerPlugin<RefinePathPlugin>();)
K_EXPORT_PLUGIN(RefinePathPluginFactory("artworkrefinepathplugin"))

RefinePathPlugin::RefinePathPlugin(QObject *parent, const QVariantList &) : Plugin(parent)
{
    QAction *actionRefinePath  = new KAction(KIcon("14_refine"), i18n("&Refine Path..."), this);
    actionCollection()->addAction("path_refine", actionRefinePath);
    connect(actionRefinePath, SIGNAL(triggered()), this, SLOT(slotRefinePath()));

    m_RefinePathDlg = new RefinePathDlg(qobject_cast<QWidget*>(parent));
}

void RefinePathPlugin::slotRefinePath()
{
    KCanvasController* canvasController = KToolManager::instance()->activeCanvasController();
    KShapeSelection *selection = canvasController->canvas()->shapeManager()->selection();
    KShape * shape = selection->firstSelectedShape();
    if (! shape)
        return;

    // check if we have a path based shape
    KPathShape * path = dynamic_cast<KPathShape*>(shape);
    if (! path)
        return;

    // check if it is no parametric shape
    KParameterShape * ps = dynamic_cast<KParameterShape*>(shape);
    if (ps && ps->isParametricShape())
        return;

    if (QDialog::Rejected == m_RefinePathDlg->exec())
        return;

    canvasController->canvas()->addCommand(new ArtworkPathRefineCommand(path, m_RefinePathDlg->knots()));
}

RefinePathDlg::RefinePathDlg(QWidget* parent, const char* name)
        : KDialog(parent)
{
    setObjectName(name);
    setModal(true);
    setCaption(i18n("Refine Path"));
    setButtons(Ok | Cancel);

    QGroupBox * group = new QGroupBox(this);
    group->setTitle(i18n("Properties"));
    setMainWidget(group);

    QHBoxLayout * hbox = new QHBoxLayout(group);
    hbox->addWidget(new QLabel(i18n("Subdivisions:"), group));

    m_knots = new KIntSpinBox(group);
    m_knots->setMinimum(1);
    hbox->addWidget(m_knots);
}

uint RefinePathDlg::knots() const
{
    return m_knots->value();
}

void RefinePathDlg::setKnots(uint value)
{
    m_knots->setValue(value);
}

#include "RefinePathPlugin.moc"
