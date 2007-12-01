/* This file is part of the KDE project
 * Copyright (C) 2007 Martin Pfeiffer <hubipete@gmx.net>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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

#include "DefaultToolWidget.h"

#include <KoInteractionTool.h>
#include <KoCanvasBase.h>
#include <KoCanvasResourceProvider.h>
#include <KoShapeManager.h>
#include <KoSelection.h>
#include <commands/KoShapeMoveCommand.h>
#include <commands/KoShapeSizeCommand.h>
#include <commands/KoShapeTransformCommand.h>

#include <QPainter>
#include <QSize>
#include <QtGui/QRadioButton>
#include <QtGui/QLabel>

DefaultToolWidget::DefaultToolWidget( KoInteractionTool* tool,
    QWidget* parent ) : QTabWidget( parent ),
    m_moveCommand(0)
{
    m_tool = tool;

    setupUi( this );

    setUnit( m_tool->canvas()->unit() );

    connect( positionSelector, SIGNAL( positionSelected(KoFlake::Position) ), this, SLOT( updatePosition() ) );

    connect( positionXSpinBox, SIGNAL( valueChanged(double) ), this, SLOT( positionHasChanged() ) );
    connect( positionYSpinBox, SIGNAL( valueChanged(double) ), this, SLOT( positionHasChanged() ) );

    connect( widthSpinBox, SIGNAL( editingFinished() ), this, SLOT( sizeHasChanged() ) );
    connect( heightSpinBox, SIGNAL( editingFinished() ), this, SLOT( sizeHasChanged() ) );

    KoSelection * selection = m_tool->canvas()->shapeManager()->selection();
    connect( selection, SIGNAL( selectionChanged() ), this, SLOT( selectionChanged() ) );
    KoShapeManager * manager = m_tool->canvas()->shapeManager();
    connect( manager, SIGNAL( selectionContentChanged() ), this, SLOT( updatePosition() ) );
    connect( manager, SIGNAL( selectionContentChanged() ), this, SLOT( updateSize() ) );

    connect( m_tool->canvas()->resourceProvider(), SIGNAL( resourceChanged( int, const QVariant& ) ),
        this, SLOT( resourceChanged( int, const QVariant& ) ) );

    bringToFront->setDefaultAction( m_tool->action( "object_move_totop" ) );
    raiseLevel->setDefaultAction( m_tool->action( "object_move_up" ) );
    lowerLevel->setDefaultAction( m_tool->action( "object_move_down" ) );
    sendBack->setDefaultAction( m_tool->action( "object_move_tobottom" ) );
    bottomAlign->setDefaultAction( m_tool->action( "object_align_vertical_bottom" ) );
    vCenterAlign->setDefaultAction( m_tool->action( "object_align_vertical_center" ) );
    topAlign->setDefaultAction( m_tool->action( "object_align_vertical_top" ) );
    rightAlign->setDefaultAction( m_tool->action( "object_align_horizontal_right" ) );
    hCenterAlign->setDefaultAction( m_tool->action( "object_align_horizontal_center" ) );
    leftAlign->setDefaultAction( m_tool->action( "object_align_horizontal_left" ) );

    aspectButton->setKeepAspectRatio( false );

    updatePosition();
    updateSize();
}

void DefaultToolWidget::updatePosition()
{
    QPointF selPosition( 0, 0 );
    KoFlake::Position position = positionSelector->position();

    KoSelection * selection = m_tool->canvas()->shapeManager()->selection();
    if( m_shapesToModify.count() )
        selPosition = selection->absolutePosition( position );

    positionXSpinBox->setEnabled( m_shapesToModify.count() );
    positionYSpinBox->setEnabled( m_shapesToModify.count() );

    positionXSpinBox->blockSignals(true);
    positionYSpinBox->blockSignals(true);
    positionXSpinBox->changeValue( selPosition.x() );
    positionYSpinBox->changeValue( selPosition.y() );
    positionXSpinBox->blockSignals(false);
    positionYSpinBox->blockSignals(false);

    emit hotPositionChanged( position );
}

void DefaultToolWidget::positionHasChanged()
{
    KoSelection * selection = m_tool->canvas()->shapeManager()->selection();
    if( ! m_shapesToModify.count() )
        return;

    KoFlake::Position position = positionSelector->position();
    QPointF newPos( positionXSpinBox->value(), positionYSpinBox->value() );
    QPointF oldPos = selection->absolutePosition( position );
    if( oldPos == newPos )
        return;

    QPointF moveBy = newPos - oldPos;
    QList<QPointF> oldPositions;
    QList<QPointF> newPositions;
    foreach( KoShape* shape, m_shapesToModify )
    {
        oldPositions.append( shape->position() );
        newPositions.append( shape->position() + moveBy );
    }
    selection->setPosition( selection->position() + moveBy );
    QTime now = QTime::currentTime();
    if (m_moveCommand ==0 || now > m_commandExpire) {
        m_moveCommand = new KoShapeMoveCommand( m_shapesToModify, oldPositions, newPositions );
        m_tool->canvas()->addCommand( m_moveCommand );
    }
    else {
        m_moveCommand->setNewPositions(newPositions);
        m_moveCommand->redo();
    }
    m_commandExpire = now.addMSecs(4000);
    updatePosition();
}

void DefaultToolWidget::selectionChanged()
{
    m_moveCommand = 0;
    m_shapesToModify.clear();
    KoSelection * selection = m_tool->canvas()->shapeManager()->selection();
    foreach(KoShape *shape, selection->selectedShapes( KoFlake::TopLevelSelection) ) {
        if (shape->isEditable())
            m_shapesToModify.append(shape);
    }
    updatePosition();
    updateSize();
}

void DefaultToolWidget::updateSize()
{
    QSizeF selSize( 0, 0 );
    KoSelection * selection = m_tool->canvas()->shapeManager()->selection();
    uint selectionCount = m_shapesToModify.count();
    if( selectionCount )
        selSize = selection->boundingRect().size();

    widthSpinBox->setEnabled( selectionCount );
    heightSpinBox->setEnabled( selectionCount );
    aspectButton->setEnabled( selectionCount );

    widthSpinBox->blockSignals(true);
    heightSpinBox->blockSignals(true);
    widthSpinBox->changeValue( selSize.width() );
    heightSpinBox->changeValue( selSize.height() );
    widthSpinBox->blockSignals(false);
    heightSpinBox->blockSignals(false);
}

void DefaultToolWidget::sizeHasChanged()
{
    QSizeF newSize( widthSpinBox->value(), heightSpinBox->value() );

    KoSelection *selection = m_tool->canvas()->shapeManager()->selection();
    QRectF rect = selection->boundingRect();

    if( aspectButton->keepAspectRatio() )
    {
        double aspect = rect.width() / rect.height();
        if( rect.width() != newSize.width() )
            newSize.setHeight( newSize.width() / aspect );
        else if( rect.height() != newSize.height() )
            newSize.setWidth( newSize.height() * aspect );
    }

    if( rect.width() != newSize.width() || rect.height() != newSize.height() )
    {
        QMatrix resizeMatrix;
        resizeMatrix.translate( rect.x(), rect.y() );
        resizeMatrix.scale( newSize.width() / rect.width(), newSize.height() / rect.height() );
        resizeMatrix.translate( -rect.x(), -rect.y() );

        QList<QSizeF> oldSizes, newSizes;
        QList<QMatrix> oldState;
        QList<QMatrix> newState;

        foreach( KoShape* shape, m_shapesToModify )
        {
            QSizeF oldSize = shape->size();
            oldState << shape->transformation();
            QMatrix shapeMatrix = shape->absoluteTransformation(0);

            // calculate the matrix we would apply to the local shape matrix
            // that tells us the effective scale values we have to use for the resizing
            QMatrix localMatrix = shapeMatrix * resizeMatrix * shapeMatrix.inverted();
            // save the effective scale values
            double scaleX = localMatrix.m11();
            double scaleY = localMatrix.m22();

            // calculate the scale matrix which is equivalent to our resizing above
            QMatrix scaleMatrix = (QMatrix().scale( scaleX, scaleY ));
            scaleMatrix =  shapeMatrix.inverted() * scaleMatrix * shapeMatrix;

            // calculate the new size of the shape, using the effective scale values
            oldSizes << oldSize;
            newSizes << QSizeF( scaleX * oldSize.width(), scaleY * oldSize.height() );
            // apply the rest of the transformation without the resizing part
            shape->applyAbsoluteTransformation( scaleMatrix.inverted() * resizeMatrix );
            newState << shape->transformation();
        }
        m_tool->repaintDecorations();
        selection->applyAbsoluteTransformation( resizeMatrix );
        QUndoCommand * cmd = new QUndoCommand(i18n("Resize"));
        new KoShapeSizeCommand( m_shapesToModify, oldSizes, newSizes, cmd );
        new KoShapeTransformCommand( m_shapesToModify, oldState, newState, cmd );
        m_tool->canvas()->addCommand( cmd );
        updateSize();
        updatePosition();
    }
}

void DefaultToolWidget::setUnit( const KoUnit &unit )
{
    // TODO find a way to get notified whenever the unit changes
    positionXSpinBox->setUnit( unit );
    positionYSpinBox->setUnit( unit );
    widthSpinBox->setUnit( unit );
    heightSpinBox->setUnit( unit );
}

void DefaultToolWidget::resourceChanged( int key, const QVariant &)
{
    if( key == KoCanvasResource::Unit )
        setUnit( m_tool->canvas()->unit() );
}

#include <DefaultToolWidget.moc>
