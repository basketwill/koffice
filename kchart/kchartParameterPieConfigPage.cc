/*
 * Copyright 2001 by Laurent Montel, released under Artistic License.
 */

#include "kchartParameterPieConfigPage.h"

#include "kchartParameterPieConfigPage.moc"

#include <kapplication.h>
#include <klocale.h>
#include <kdebug.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qspinbox.h>

#include "kchart_params.h"

KChartParameterPieConfigPage::KChartParameterPieConfigPage( KChartParams* params,
                                                            QWidget* parent ) :
    QWidget( parent ),_params( params )
{
    QGridLayout *grid1 = new QGridLayout(this,2,2,15,7);

    QGroupBox* gb2 = new QGroupBox( i18n( "Title" ), this );
    QGridLayout *grid3 = new QGridLayout(gb2,4,2,15,7);

    QLabel* label = new QLabel( i18n( "Title" ), gb2 );
    label->resize( label->sizeHint() );
    label->setAlignment(Qt::AlignCenter);
    grid3->addWidget( label,0,0 );

    title = new QLineEdit( gb2 );
    title->resize(100, title->sizeHint().height() );
    grid3->addWidget( title,1,0 );
    label->setBuddy( title );

    grid3->activate();
    grid1->addWidget(gb2,0,0);

}

void KChartParameterPieConfigPage::init()
{
    title->setText(_params->header1Text());
}


void KChartParameterPieConfigPage::apply()
{
     _params->setHeader1Text(title->text());
}
