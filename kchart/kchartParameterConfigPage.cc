/*
 * $Id$
 *
 * Copyright 2000 by Laurent Montel, released under Artistic License.
 */

#include "kchartParameterConfigPage.h"

#include "kchartParameterConfigPage.moc"

#include <kapp.h>
#include <klocale.h>


#include <qlayout.h>
#include <qlabel.h>
#include <qbuttongroup.h>


KChartParameterConfigPage::KChartParameterConfigPage(KChartParameters* params,QWidget* parent ) :
    QWidget( parent ),_params( params )
{
    QVBoxLayout* toplevel = new QVBoxLayout( this, 10 );

    QGridLayout* layout = new QGridLayout( 1, 2 );
    toplevel->addLayout( layout );

    QButtonGroup* gb1 = new QButtonGroup( i18n("Parameters"), this );
    QGridLayout *grid1 = new QGridLayout(gb1,7,1,15,7);

    grid = new QCheckBox( i18n( "Grid" ), gb1 );
    grid1->addWidget(grid,0,0);

    border = new QCheckBox( i18n( "Border" ), gb1 );
    grid1->addWidget(border,1,0);

    xaxis = new QCheckBox( i18n( "X-Axis" ), gb1 );
    grid1->addWidget(xaxis,2,0);

    yaxis = new QCheckBox( i18n( "Y-Axis" ), gb1);
    grid1->addWidget(yaxis,3,0);

    shelf = new QCheckBox( i18n( "Shelf grid" ), gb1 );
    grid1->addWidget(shelf,4,0);

    xlabel = new QCheckBox( i18n( "Has X-Label" ), gb1 );
    grid1->addWidget(xlabel,5,0);

    yaxis2 = new QCheckBox( i18n( "Y-Axis2" ), gb1 );
    grid1->addWidget(yaxis2,6,0);

    QButtonGroup* gb2 = new QButtonGroup( i18n("Title"), this );
    QGridLayout *grid2 = new QGridLayout(gb2,7,2,15,7);
    QLabel *tmpLabel = new QLabel( i18n( "Title" ), gb2 );
    tmpLabel->setAlignment(Qt::AlignCenter);
    grid2->addWidget(tmpLabel,0,0);

    title= new QLineEdit( gb2 );
    title->setMaximumWidth(130);
    grid2->addWidget(title,1,0);


    tmpLabel = new QLabel( i18n( "Y-Title" ), gb2 );
    tmpLabel->setAlignment(Qt::AlignCenter);
    grid2->addWidget(tmpLabel,2,0);

    ytitle= new QLineEdit( gb2 );
    ytitle->setMaximumWidth(130);
    grid2->addWidget(ytitle,3,0);

    tmpLabel = new QLabel( i18n( "X-Title" ), gb2 );
    tmpLabel->setAlignment(Qt::AlignCenter);
    grid2->addWidget(tmpLabel,4,0);

    xtitle= new QLineEdit( gb2 );
    xtitle->setMaximumWidth(130);
    grid2->addWidget(xtitle,5,0);

    tmpLabel = new QLabel( i18n( "Y-Label format" ), gb2 );
    tmpLabel->setAlignment(Qt::AlignCenter);
    grid2->addWidget(tmpLabel,0,1);

    ylabel_fmt= new QLineEdit( gb2 );
    ylabel_fmt->setMaximumWidth(130);
    grid2->addWidget(ylabel_fmt,1,1);

    tmpLabel = new QLabel( i18n( "Y-Title 2" ), gb2 );
    tmpLabel->setAlignment(Qt::AlignCenter);
    grid2->addWidget(tmpLabel,2,1);

    ytitle2= new QLineEdit( gb2 );
    ytitle2->setMaximumWidth(130);
    grid2->addWidget(ytitle2,3,1);

    tmpLabel = new QLabel( i18n( "Y-Label format 2" ), gb2 );
    tmpLabel->setAlignment(Qt::AlignCenter);
    grid2->addWidget(tmpLabel,4,1);

    ylabel2_fmt= new QLineEdit( gb2 );
    ylabel2_fmt->setMaximumWidth(130);
    grid2->addWidget(ylabel2_fmt,5,1);

    layout->addWidget(gb1,0,0);
    layout->addWidget(gb2,0,1);

    grid1->activate();
    grid2->activate();

    connect( grid, SIGNAL( toggled( bool ) ),
  		   this, SLOT( changeState( bool ) ) );
    connect( xaxis, SIGNAL( toggled( bool ) ),
  		   this, SLOT( changeXaxisState( bool ) ) );
}

void KChartParameterConfigPage::changeXaxisState(bool state)
{
if(state)
   xlabel->setEnabled(true);
else
   xlabel->setEnabled(false);

}

void KChartParameterConfigPage::changeState(bool state)
{
if(state)
   shelf->setEnabled(true);
else
   shelf->setEnabled(false);

}

void KChartParameterConfigPage::init()
{
    grid->setChecked(_params->grid);
    border->setChecked(_params->border);
    xaxis->setChecked(_params->xaxis);
    yaxis->setChecked(_params->yaxis);
    xlabel->setChecked(_params->hasxlabel);
    shelf->setChecked(_params->shelf);
    if(_params->has_yaxis2())
    	{
    	yaxis2->setChecked(_params->yaxis2);
    	if(!_params->ylabel2_fmt.isEmpty())
    		{
    		int len=_params->ylabel2_fmt.length();
         	ylabel2_fmt->setText(_params->ylabel2_fmt.right(len-3));
         	}
    	ytitle2->setText(_params->ytitle2);
    	}
    else
    	{
    	yaxis2->setEnabled(false);
    	ylabel2_fmt->setEnabled(false);
    	ytitle2->setEnabled(false);
    	}
    if(_params->grid)
    	shelf->setEnabled(true);
    else
    	shelf->setEnabled(false);
    title->setText(_params->title);
    xtitle->setText(_params->xtitle);
    ytitle->setText(_params->ytitle);
    if(!_params->ylabel_fmt.isEmpty())
    		{
    		int len=_params->ylabel_fmt.length();
         	ylabel_fmt->setText(_params->ylabel_fmt.right(len-3));
         	}
    if(_params->xaxis)
    	xlabel->setEnabled(true);
    else
    	xlabel->setEnabled(false);

}
void KChartParameterConfigPage::apply()
{
    _params->grid=grid->isChecked();
    _params->border=border->isChecked();
    _params->xaxis=xaxis->isChecked();
    _params->yaxis=yaxis->isChecked();
    if(xaxis->isChecked())
    	_params->hasxlabel=xlabel->isChecked();
    if(_params->has_yaxis2())
    	{
    	_params->yaxis2=yaxis2->isChecked();
    	if(!ylabel2_fmt->text().isEmpty())
    		_params->ylabel2_fmt="%g "+ylabel2_fmt->text();
    	_params->ytitle2=ytitle2->text();
    	}


    if(grid->isChecked())
    	_params->shelf=shelf->isChecked();
    _params->title=title->text();
    _params->xtitle=xtitle->text();
    _params->ytitle=ytitle->text();
    if(!ylabel_fmt->text().isEmpty())
    	_params->ylabel_fmt="%g "+ylabel_fmt->text();

}
