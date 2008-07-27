/* This file is part of the KDE project
 * Copyright (C) 2008 Fela Winkelmolen <fela.kde@gmail.com>
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

#include "KarbonCalligraphyTool.h"
#include "KarbonCalligraphicShape.h"
#include "KarbonCalligraphyOptionWidget.h"

#include <KoPathShape.h>
#include <KoShapeGroup.h>
#include <KoLineBorder.h>
#include <KoPointerEvent.h>
#include <KoPathPoint.h>
#include <KoCanvasBase.h>
#include <KoShapeController.h>
#include <KoShapeManager.h>
#include <KoSelection.h>
#include <KarbonCurveFit.h>
#include <KoColorBackground.h>

#include <QtGui/QPainter>

#include <cmath>

#undef M_PI
const double M_PI = 3.1415927;
using std::pow;
using std::sqrt;


KarbonCalligraphyTool::KarbonCalligraphyTool(KoCanvasBase *canvas)
    : KoTool( canvas ), m_shape( 0 ), m_isDrawing( false ), m_speed(0, 0)
{
}

KarbonCalligraphyTool::~KarbonCalligraphyTool()
{
}

void KarbonCalligraphyTool::paint( QPainter &painter,
                                   const KoViewConverter &converter )
{
    if ( ! m_shape )
        return;

    painter.save();

    painter.setMatrix( m_shape->absoluteTransformation(&converter) *
                       painter.matrix() );
    m_shape->paint( painter, converter );

    painter.restore();
}

void KarbonCalligraphyTool::mousePressEvent( KoPointerEvent *event )
{
    if ( m_isDrawing ) 
        return;

    m_lastPoint = event->point;
    m_speed = QPointF(0, 0);

    m_isDrawing = true;
    m_firstPointAdded = false;
    m_shape = new KarbonCalligraphicShape;
    //addPoint( event );
}

void KarbonCalligraphyTool::mouseMoveEvent( KoPointerEvent *event )
{
    if ( ! m_isDrawing )
        return;

    addPoint( event );
}

void KarbonCalligraphyTool::mouseReleaseEvent( KoPointerEvent *event )
{
    Q_UNUSED( event );

    if ( ! m_isDrawing )
        return;

    //addPoint( event );
    m_isDrawing = false;

    if ( m_shape->pointCount() == 0 )
    {
        delete m_shape;
        m_shape = 0;
        return;
    }

    m_shape->simplifyPath();
    KoPathShape *finalPath = m_shape;

    QUndoCommand * cmd = m_canvas->shapeController()->addShape( finalPath );
    if( cmd )
    {
        KoSelection *selection = m_canvas->shapeManager()->selection();
        selection->deselectAll();
        selection->select( finalPath );
        m_canvas->addCommand( cmd );
    }
    else
    {
        // TODO: when may this happen????
        delete finalPath;
    }

    m_canvas->updateCanvas( m_shape->boundingRect() );
    m_canvas->updateCanvas( finalPath->boundingRect() );

    m_shape = 0;
}

void KarbonCalligraphyTool::addPoint( KoPointerEvent *event )
{
    if ( ! m_firstPointAdded ) {
        m_lastPoint = event->point;
        m_firstPointAdded = true;
        return;
    }

    QPointF force = event->point - m_lastPoint;
    QPointF dSpeed = force/m_mass;
    // apply drag and "force"
    QPointF newSpeed = m_speed * (1.0 - m_drag) + dSpeed;
    double width = calculateWidth( event->pressure() );
    double angle = calculateAngle( m_speed, newSpeed );

    m_shape->appendPoint( m_lastPoint, angle, width );

    m_speed = newSpeed;
    m_lastPoint = m_lastPoint + m_speed;
    m_canvas->updateCanvas( m_shape->lastPieceBoundingRect() );
}

double KarbonCalligraphyTool::calculateWidth( double pressure )
{
    // calculate the modulo of the speed
    double speed = std::sqrt( pow(m_speed.x(), 2) + pow(m_speed.y(), 2) );
    double thinning =  m_thinning * (speed + 1) / 10.0; // can be negative

    if ( thinning > 1 )
        thinning = 1;

    if ( ! m_usePressure )
        pressure = 1.0;

    double strokeWidth = m_strokeWidth * pressure * (1 - thinning);

    const double MINIMUM_STROKE_WIDTH = 1.0;
    if ( strokeWidth < MINIMUM_STROKE_WIDTH )
        strokeWidth = MINIMUM_STROKE_WIDTH;

    return strokeWidth;
}


double KarbonCalligraphyTool::calculateAngle( const QPointF &oldSpeed,
                                              const QPointF &newSpeed )
{
    // calculate the avarage of the speed (sum of the normalized values)
    double oldLength = QLineF( QPointF(0,0), oldSpeed ).length();
    double newLength = QLineF( QPointF(0,0), newSpeed ).length();
    QPointF oldSpeedNorm = oldLength ? oldSpeed/oldLength : QPointF(0, 0);
    QPointF newSpeedNorm = newLength ? newSpeed/newLength : QPointF(0, 0);
    QPointF speed = oldSpeedNorm + newSpeedNorm;

    // angle solely based on the speed
    double speedAngle = 0;
    if ( speed.x() != 0 ) // avoid division by zero
    {
        speedAngle = std::atan( speed.y() / speed.x() );
    }
    else if ( speed.y() != 0 )
    {
        // x == 0 && y != 0
        speedAngle = M_PI;
    }
    if ( speed.x() < 0 )
        speedAngle += M_PI;

    // move 90 degrees
    speedAngle += M_PI/2;

    double fixedAngle = m_angle;
    // check if the fixed angle needs to be flipped
    double diff = fixedAngle - speedAngle;
    while ( diff >= M_PI ) // normalize diff between -180 and 180
        diff -= 2*M_PI;
    while ( diff < -M_PI )
        diff += 2*M_PI;

    if ( std::abs(diff) > M_PI/2 ) // if absolute value < 90
        fixedAngle += M_PI; // += 180
    
    double dAngle = speedAngle - fixedAngle;

    // normalize dAngle between -90 and +90
    while ( dAngle >= M_PI/2 )
        dAngle -= M_PI;
    while ( dAngle < -M_PI/2 )
        dAngle += M_PI;

    double angle = fixedAngle + dAngle*(1.0 - m_fixation);

    return angle;
}

void KarbonCalligraphyTool::activate( bool )
{
    useCursor(Qt::ArrowCursor, true);
}

void KarbonCalligraphyTool::deactivate()
{
}

QWidget *KarbonCalligraphyTool::createOptionWidget()
{
    KarbonCalligraphyOptionWidget *widget = new KarbonCalligraphyOptionWidget;

    connect( widget, SIGNAL(usePathChanged(bool)),
            this, SLOT(setUsePath(bool)));

    connect( widget, SIGNAL(usePressureChanged(bool)),
            this, SLOT(setUsePressure(bool)));

    connect( widget, SIGNAL(useAngleChanged(bool)),
            this, SLOT(setUseAngle(bool)));

    connect( widget, SIGNAL(widthChanged(double)),
             this, SLOT(setStrokeWidth(double)));

    connect( widget, SIGNAL(thinningChanged(double)),
             this, SLOT(setThinning(double)));

    connect( widget, SIGNAL(angleChanged(int)),
             this, SLOT(setAngle(int)));

    connect( widget, SIGNAL(fixationChanged(double)),
             this, SLOT(setFixation(double)));

    connect( widget, SIGNAL(capsChanged(double)),
             this, SLOT(setCaps(double)));

    connect( widget, SIGNAL(massChanged(double)),
             this, SLOT(setMass(double)));

    connect( widget, SIGNAL(dragChanged(double)),
             this, SLOT(setDrag(double)));

    widget->emitAll();

    return widget;
}

void KarbonCalligraphyTool::setStrokeWidth( double width )
{
    m_strokeWidth = width;
}

void KarbonCalligraphyTool::setThinning( double thinning )
{
    m_thinning = thinning/2.0;
}

void KarbonCalligraphyTool::setAngle( int angle )
{
    m_angle = angle/180.0*M_PI;
}

void KarbonCalligraphyTool::setFixation( double fixation )
{
    m_fixation = fixation;
}

void KarbonCalligraphyTool::setMass( double mass )
{
    m_mass = mass * mass + 1;
}

void KarbonCalligraphyTool::setDrag( double drag )
{
    m_drag = drag;
}

void KarbonCalligraphyTool::setUsePath( bool usePath )
{
    m_usePath = usePath;
}

void KarbonCalligraphyTool::setUsePressure( bool usePressure )
{
    m_usePressure = usePressure;
}

void KarbonCalligraphyTool::setUseAngle(bool useAngle )
{
    m_useAngle = useAngle;
}

void KarbonCalligraphyTool::setCaps( double caps )
{
    m_caps = caps;
}

