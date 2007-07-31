/* This file is part of the KDE project
   Copyright (C) 1999,2000 Matthias Kalle Dalheimer <kalle@kde.org>

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

#include <stdlib.h>

#include <qradiobutton.h>
#include <QSpinBox>
#include <QLabel>
#include <QLayout>

#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>


#include <QLayout>
#include <QButtonGroup>
#include <QGroupBox>
//Added by qt3to4:
#include <Q3VButtonGroup>

#include "kchart_params.h"
#include "kchart_factory.h"

#include "KCConfigSubtypePage.h"


namespace KChart
{

KChartHiloSubTypeChartPage::KChartHiloSubTypeChartPage( KChartParams* params,
                                                        QWidget* parent ) :
    KCConfigSubtypePage(  params, parent )
{
    QHBoxLayout* toplevel = new QHBoxLayout( this );
    toplevel->setMargin( 10 );

    // The left side: The radio buttons
    QGroupBox* subtypeGB = new QGroupBox( i18n( "Sub-type" ) );
    subtypeGB->setWhatsThis( i18n("Select the desired sub-type of a chart. The available sub-types depend on the chart type. Some chart types have no sub-type at all, in which case this configuration page is not shown."));
    toplevel->addWidget( subtypeGB );

    QVBoxLayout  *layout = new QVBoxLayout( subtypeGB );
    layout->setMargin( 10 );

    QButtonGroup* subtypeBG = new QButtonGroup( );
    subtypeBG->setExclusive( true );

    normal = new QRadioButton( i18n( "Normal" ), subtypeGB );
    layout->addWidget( normal );
    subtypeBG->addButton( normal, KDChartParams::HiLoNormal );

    stacked = new QRadioButton(i18n("HiLoClose"), subtypeGB );
    layout->addWidget( stacked );
    subtypeBG->addButton( stacked, KDChartParams::HiLoClose );

    percent = new QRadioButton( i18n("HiLoOpenClose"), subtypeGB );
    layout->addWidget( percent );
    subtypeBG->addButton( percent, KDChartParams::HiLoOpenClose );

    layout->addStretch( 1 );
    subtypeGB->setFixedWidth( subtypeGB->sizeHint().width() );

    // Notify when the user clicks one of the radio buttons.
    connect( subtypeBG, SIGNAL( buttonClicked( int ) ),
             this,      SLOT( slotChangeSubType( int ) ) );

    // The right side: The example icon.
    QGroupBox* exampleGB = new QGroupBox(i18n( "Example" ) );
    exampleGB->setWhatsThis( i18n("Preview the sub-type you choose."));
    toplevel->addWidget( exampleGB, 2 );

    QHBoxLayout *layout2 = new QHBoxLayout( exampleGB );
    layout2->setMargin( 10 );

    exampleLA = new QLabel( exampleGB );
    exampleLA->setAlignment( Qt::AlignCenter | Qt::AlignVCenter );
    layout2->addWidget( exampleLA );
}

void KChartHiloSubTypeChartPage::init()
{
    switch( m_params->hiLoChartSubType() ) {
    case KDChartParams::HiLoNormal:
        normal->setChecked( true );
        break;
    case KDChartParams::HiLoClose:
        stacked->setChecked( true );
        break;
    case KDChartParams::HiLoOpenClose:
        percent->setChecked( true );
        break;
    default:
        {
            kDebug( 35001 ) <<"Error in stack_type";
            abort();
            break;
        }
    }

    slotChangeSubType( m_params->hiLoChartSubType() );
}

void KChartHiloSubTypeChartPage::slotChangeSubType( int type )
{
    switch( type ) {
    case KDChartParams::HiLoNormal:
        exampleLA->setPixmap( UserIcon( "chart_hilo_normal" /*, KChartFactory::global()*/  ) );
        break;
    case KDChartParams::HiLoClose:
        exampleLA->setPixmap( UserIcon( "chart_hilo_close" ) );
        break;
    case KDChartParams::HiLoOpenClose:
        exampleLA->setPixmap( UserIcon( "chart_hilo_openclose" ) );
        break;
    };
}



void KChartHiloSubTypeChartPage::apply()
{
    if( normal->isChecked() )
	m_params->setHiLoChartSubType( KDChartParams::HiLoNormal );
    else if( stacked->isChecked() )
        m_params->setHiLoChartSubType( KDChartParams::HiLoClose );
    else if( percent->isChecked() )
        m_params->setHiLoChartSubType( KDChartParams::HiLoOpenClose );
    else {
        kDebug( 35001 ) <<"Error in groupbutton";
    }
}

KChartAreaSubTypeChartPage::KChartAreaSubTypeChartPage( KChartParams* params,
                                                        QWidget* parent ) :
    KCConfigSubtypePage(  params, parent )
{
    QHBoxLayout* toplevel = new QHBoxLayout( this );
    toplevel->setMargin( 10 );

    QGroupBox* box = new QGroupBox( i18n( "Sub-type" ) );
    box->setWhatsThis( i18n("Select the desired sub-type of a chart. The available sub-types depend on the chart type. Some chart types have no sub-type at all, in which case this configuration page is not shown."));
    toplevel->addWidget( box );

    QVBoxLayout  *layout = new QVBoxLayout( box );
    layout->setMargin( 10 );

    QButtonGroup* subtypeBG = new QButtonGroup( );
    subtypeBG->setExclusive( true );

    normal = new QRadioButton( i18n( "Normal" ), box );
    layout->addWidget( normal );
    subtypeBG->addButton( normal, KDChartParams::AreaNormal );

    stacked = new QRadioButton( i18n( "Stacked" ), box );
    layout->addWidget( stacked );
    subtypeBG->addButton( stacked, KDChartParams::AreaStacked );

    percent = new QRadioButton( i18n( "Percent" ), box );
    layout->addWidget( percent );
    subtypeBG->addButton( percent, KDChartParams::AreaPercent );

    layout->addStretch( 1 );
    box->setFixedWidth( box->sizeHint().width() );

    // Notify when the user clicks one of the radio buttons.
    connect( subtypeBG, SIGNAL( buttonClicked( int ) ),
             this,      SLOT( slotChangeSubType( int ) ) );

    QGroupBox* exampleGB = new QGroupBox( i18n( "Example" ), this );
    exampleGB->setWhatsThis( i18n("Preview the sub-type you choose."));
    toplevel->addWidget( exampleGB, 2 );

    QHBoxLayout *layout2 = new QHBoxLayout( exampleGB );
    layout2->setMargin( 10 );

    exampleLA = new QLabel( exampleGB );
    exampleLA->setAlignment( Qt::AlignCenter | Qt::AlignVCenter );
    layout2->addWidget( exampleLA );
}


void KChartAreaSubTypeChartPage::init()
{
    switch( m_params->areaChartSubType() ) {
    case KDChartParams::AreaNormal:
        normal->setChecked( true );
        break;
    case KDChartParams::AreaStacked:
        stacked->setChecked( true );
        break;
    case KDChartParams::AreaPercent:
        percent->setChecked( true );
        break;
    default:
        {
            kDebug( 35001 ) <<"Error in stack_type";
            abort();
            break;
        }
    }

    slotChangeSubType( m_params->areaChartSubType() );
}

void KChartAreaSubTypeChartPage::slotChangeSubType( int type )
{
    switch( type ) {
    case KDChartParams::AreaNormal:
        exampleLA->setPixmap( UserIcon( "chart_area_normal" ) );
        break;
    case KDChartParams::AreaStacked:
        exampleLA->setPixmap( UserIcon( "chart_area_stacked" ) );
        break;
    case KDChartParams::AreaPercent:
        exampleLA->setPixmap( UserIcon( "chart_area_percent" ) );
        break;
    };
}



void KChartAreaSubTypeChartPage::apply()
{
    if( normal->isChecked() )
        m_params->setAreaChartSubType( KDChartParams::AreaNormal );
    else if( stacked->isChecked() )
        m_params->setAreaChartSubType( KDChartParams::AreaStacked );
    else if( percent->isChecked() )
        m_params->setAreaChartSubType( KDChartParams::AreaPercent );
    else {
        kDebug( 35001 ) <<"Error in groupbutton";
    }
}

KChartBarSubTypeChartPage::KChartBarSubTypeChartPage( KChartParams* params,
                                                      QWidget* parent ) :
    KCConfigSubtypePage( params, parent )
{
    QHBoxLayout* toplevel = new QHBoxLayout( this );
    toplevel->setMargin( 10 );

    // The left side including the subtype and number of lines.
    //
    // NOTE: This uses a layout directly in a layout (leftLayout as
    //       the leftmost element of toplevel.
    QVBoxLayout  *leftLayout = new QVBoxLayout( );
    toplevel->addLayout( leftLayout );

    QGroupBox *box = new QGroupBox( i18n( "Sub-type" ) );
    box->setWhatsThis( i18n("Select the desired sub-type of a chart. The available sub-types depend on the chart type. Some chart types have no sub-type at all, in which case this configuration page is not shown."));
    leftLayout->addWidget( box );

    QVBoxLayout  *boxLayout = new QVBoxLayout( box );
    boxLayout->setMargin( 10 );

    QButtonGroup* subtypeBG = new QButtonGroup( );
    subtypeBG->setExclusive( true );

    normal = new QRadioButton( i18n( "Normal" ), box );
    boxLayout->addWidget( normal );
    subtypeBG->addButton( normal, KDChartParams::BarNormal );

    stacked = new QRadioButton( i18n( "Stacked" ), box );
    boxLayout->addWidget( stacked );
    subtypeBG->addButton( stacked, KDChartParams::BarStacked );

    percent = new QRadioButton( i18n( "Percent" ), box );
    boxLayout->addWidget( percent );
    subtypeBG->addButton( percent, KDChartParams::BarPercent );

    // Notify when the user clicks one of the radio buttons.
    connect( subtypeBG, SIGNAL( buttonClicked( int ) ),
             this,      SLOT( slotChangeSubType( int ) ) );

    QLabel  *lbl = new QLabel( i18n( "Number of lines: ") );
    leftLayout->addWidget( lbl );
    m_numLines    = new QSpinBox( );
    leftLayout->addWidget( m_numLines );
    leftLayout->addStretch( 1 );

    // ------------------------------------
    // The right side with the example icon
    QGroupBox* exampleGB = new QGroupBox( i18n( "Example" ) );
    exampleGB->setWhatsThis( i18n("Preview the sub-type you choose."));
    toplevel->addWidget( exampleGB, 2 );

    QHBoxLayout *layout2 = new QHBoxLayout( exampleGB );
    layout2->setMargin( 10 );

    exampleLA = new QLabel( exampleGB );
    exampleLA->setAlignment( Qt::AlignCenter | Qt::AlignVCenter );
    layout2->addWidget( exampleLA );
}

void KChartBarSubTypeChartPage::init()
{
    // SUM is for areas only and therefore not configurable here.
    switch( m_params->barChartSubType() ) {
    case KDChartParams::BarNormal:
	normal->setChecked( true );
        break;
    case KDChartParams::BarStacked:
        stacked->setChecked( true );
        break;
    case KDChartParams::BarPercent:
        percent->setChecked( true );
        break;
    default:
        {
            kDebug( 35001 ) <<"Error in stack_type";
            break;
        }
    }

    m_numLines->setValue( m_params->barNumLines() );

    slotChangeSubType( m_params->barChartSubType() );
}


void KChartBarSubTypeChartPage::slotChangeSubType( int type )
{
    switch( type ) {
    case KDChartParams::BarStacked:
	exampleLA->setPixmap( UserIcon( "chart_bar_layer" ) );
	break;
    case KDChartParams::BarNormal:
	exampleLA->setPixmap( UserIcon( "chart_bar_beside" ) );
	break;
    case KDChartParams::BarPercent:
	exampleLA->setPixmap( UserIcon( "chart_bar_percent" ) );
	break;
    };
}


void KChartBarSubTypeChartPage::apply()
{
    if( normal->isChecked() ) {
        m_params->setBarChartSubType( KDChartParams::BarNormal );
    } else if( stacked->isChecked() ) {
        m_params->setBarChartSubType( KDChartParams::BarStacked );
    } else if( percent->isChecked() )	{
        m_params->setBarChartSubType( KDChartParams::BarPercent );
    } else {
        kDebug( 35001 ) <<"Error in groupbutton";
    }

    // FIXME: Error controls.
    m_params->setBarNumLines( m_numLines->value() );
}

KChartLineSubTypeChartPage::KChartLineSubTypeChartPage( KChartParams* params,
                                                        QWidget* parent ) :
    KCConfigSubtypePage(  params, parent )
{
    QHBoxLayout* toplevel = new QHBoxLayout( this );
    toplevel->setMargin( 10 );

    QGroupBox* box = new QGroupBox( i18n( "Sub-type" ) );
    box->setWhatsThis( i18n("Select the desired sub-type of a chart. The available sub-types depend on the chart type. Some chart types have no sub-type at all, in which case this configuration page is not shown."));
    toplevel->addWidget( box );

    QVBoxLayout  *layout = new QVBoxLayout( box );
    layout->setMargin( 10 );

    QButtonGroup* subtypeBG = new QButtonGroup( );
    subtypeBG->setExclusive( true );

    normal = new QRadioButton( i18n( "Normal" ), box );
    layout->addWidget( normal );
    subtypeBG->addButton( normal, KDChartParams::LineNormal );

    stacked = new QRadioButton( i18n( "Stacked" ), box );
    layout->addWidget( stacked );
    subtypeBG->addButton( stacked, KDChartParams::LineStacked );

    percent = new QRadioButton( i18n( "Percent" ), box );
    layout->addWidget( percent );
    subtypeBG->addButton( percent, KDChartParams::LinePercent );

    layout->addStretch( 1 );
    box->setFixedWidth( box->sizeHint().width() );

    // Notify when the user clicks one of the radio buttons.
    connect( subtypeBG, SIGNAL( buttonClicked( int ) ),
             this,      SLOT( slotChangeSubType( int ) ) );

    QGroupBox* exampleGB = new QGroupBox( i18n( "Example" ) );
    exampleGB->setWhatsThis( i18n("Preview the sub-type you choose."));
    toplevel->addWidget( exampleGB, 2 );

    QHBoxLayout *layout2 = new QHBoxLayout( exampleGB );
    layout2->setMargin( 10 );

    exampleLA = new QLabel( exampleGB );
    exampleLA->setAlignment( Qt::AlignCenter | Qt::AlignVCenter );
    layout2->addWidget( exampleLA );
}

void KChartLineSubTypeChartPage::init()
{
    switch( m_params->lineChartSubType() ) {
    case KDChartParams::LineNormal:
        normal->setChecked( true );
        break;
    case KDChartParams::LineStacked:
        stacked->setChecked( true );
        break;
    case KDChartParams::LinePercent:
        percent->setChecked( true );
        break;
    default:
        {
            kDebug( 35001 ) <<"Error in stack_type";
            abort();
            break;
        }
    }

    slotChangeSubType( m_params->lineChartSubType() );
}

void KChartLineSubTypeChartPage::slotChangeSubType( int type )
{
    switch( type ) {
    case KDChartParams::AreaNormal:
        exampleLA->setPixmap( UserIcon( "chart_line_normal"  ) );
        break;
    case KDChartParams::AreaStacked:
        exampleLA->setPixmap( UserIcon( "chart_line_stacked" ) );
        break;
    case KDChartParams::AreaPercent:
        exampleLA->setPixmap( UserIcon( "chart_line_percent" ) );
        break;
    };
}



void KChartLineSubTypeChartPage::apply()
{
    if( normal->isChecked() )
        m_params->setLineChartSubType( KDChartParams::LineNormal );
    else if( stacked->isChecked() )
        m_params->setLineChartSubType( KDChartParams::LineStacked );
    else if( percent->isChecked() )
        m_params->setLineChartSubType( KDChartParams::LinePercent );
    else {
        kDebug( 35001 ) <<"Error in groupbutton";
    }
}

KChartPolarSubTypeChartPage::KChartPolarSubTypeChartPage( KChartParams* params,
                                                        QWidget* parent ) :
    KCConfigSubtypePage(  params, parent )
{
    QHBoxLayout* toplevel = new QHBoxLayout( this );
    toplevel->setMargin( 10 );

    Q3VButtonGroup* subtypeBG = new Q3VButtonGroup( i18n( "Sub-type" ), this );
    subtypeBG->setWhatsThis( i18n("Select the desired sub-type of a chart. The available sub-types depend on the chart type. Some chart types have no sub-type at all, in which case this configuration page is not shown."));
    toplevel->addWidget( subtypeBG, Qt::AlignCenter| Qt::AlignVCenter );
    normal = new QRadioButton( i18n( "Normal" ), subtypeBG );
    subtypeBG->insert( normal, KDChartParams::AreaNormal );
    stacked = new QRadioButton( i18n( "Stacked" ), subtypeBG );
    subtypeBG->insert( stacked, KDChartParams::AreaStacked );
    percent = new QRadioButton( i18n( "Percent" ), subtypeBG );
    subtypeBG->insert( percent, KDChartParams::AreaPercent );
    subtypeBG->setFixedWidth( subtypeBG->sizeHint().width() );
    connect( subtypeBG, SIGNAL( clicked( int ) ),
             this, SLOT( slotChangeSubType( int ) ) );

    QGroupBox* exampleGB = new QGroupBox( i18n( "Example" ) );
    exampleGB->setWhatsThis( i18n("Preview the sub-type you choose."));
    toplevel->addWidget( exampleGB, 2 );

    QHBoxLayout *layout2 = new QHBoxLayout( exampleGB );
    layout2->setMargin( 10 );

    exampleLA = new QLabel( exampleGB );
    exampleLA->setAlignment( Qt::AlignCenter | Qt::AlignVCenter );
    layout2->addWidget( exampleLA );
}

void KChartPolarSubTypeChartPage::init()
{
    switch( m_params->polarChartSubType() ) {
    case KDChartParams::PolarNormal:
        normal->setChecked( true );
        break;
    case KDChartParams::PolarStacked:
        stacked->setChecked( true );
        break;
    case KDChartParams::PolarPercent:
        percent->setChecked( true );
        break;
    default:
        {
            kDebug( 35001 ) <<"Error in stack_type";
            abort();
            break;
        }
    }

    slotChangeSubType( m_params->lineChartSubType() );
}

void KChartPolarSubTypeChartPage::slotChangeSubType( int type )
{
    switch( type ) {
    case KDChartParams::PolarNormal:
        exampleLA->setPixmap( UserIcon( "chart_polar_normal" ) );
        break;
    case KDChartParams::PolarStacked:
        exampleLA->setPixmap( UserIcon( "chart_polar_stacked" ) );
        break;
    case KDChartParams::PolarPercent:
        exampleLA->setPixmap( UserIcon( "chart_polar_percent" ) );
        break;
    };
}



void KChartPolarSubTypeChartPage::apply()
{
    if( normal->isChecked() )
        m_params->setPolarChartSubType( KDChartParams::PolarNormal );
    else if( stacked->isChecked() )
        m_params->setPolarChartSubType( KDChartParams::PolarStacked );
    else if( percent->isChecked() )
        m_params->setPolarChartSubType( KDChartParams::PolarPercent );
    else {
        kDebug( 35001 ) <<"Error in groupbutton";
    }
}

}  //KChart namespace

#include "KCConfigSubtypePage.moc"
