/* This file is part of the KDE project
   Copyright 2007 Montel Laurent <montel@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <QPainter>
#include <QGridLayout>
#include <QToolButton>

#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <KFileDialog>

#include <KoCanvasBase.h>
#include <KoSelection.h>
#include <KoShapeManager.h>

#include "PictureShape.h"

#include "PictureTool.h"
#include "PictureTool.moc"

PictureTool::PictureTool( KoCanvasBase* canvas )
    : KoTool( canvas ),
      m_pictureshape(0)
{
}

PictureTool::~PictureTool()
{
}

void PictureTool::activate (bool temporary)
{
    Q_UNUSED( temporary );
    kDebug() << k_funcinfo << endl;

    KoSelection* selection = m_canvas->shapeManager()->selection();
    foreach ( KoShape* shape, selection->selectedShapes() )
    {
        m_pictureshape = dynamic_cast<PictureShape*>( shape );
        if ( m_pictureshape )
            break;
    }
    if ( !m_pictureshape )
    {
        emit sigDone();
        return;
    }
    useCursor( Qt::ArrowCursor, true );
}

void PictureTool::deactivate()
{
  kDebug()<<"PictureTool::deactivate\n";
  m_pictureshape = 0;
}

void PictureTool::paint( QPainter& painter, KoViewConverter& viewConverter )
{
}

void PictureTool::mousePressEvent( KoPointerEvent* )
{
}

void PictureTool::mouseMoveEvent( KoPointerEvent* )
{
}

void PictureTool::mouseReleaseEvent( KoPointerEvent* )
{
}


QWidget * PictureTool::createOptionWidget()
{
    QWidget *optionWidget = new QWidget();
    QGridLayout *layout = new QGridLayout( optionWidget );

    QToolButton *button = 0;

    button = new QToolButton( optionWidget );
    button->setIcon( SmallIcon("open") );
    button->setToolTip( i18n( "Open" ) );
    layout->addWidget( button, 0, 0 );
    connect( button, SIGNAL( clicked( bool ) ), this, SLOT( slotChangeUrl() ) );

    return optionWidget;

}

void PictureTool::slotChangeUrl()
{
  kDebug()<<" PictureTool::slotChangeUrl \n";
  KUrl url = KFileDialog::getOpenUrl();
  if(!url.isEmpty() && m_pictureshape)
    m_pictureshape->setCurrentUrl(url);
}

