#include "kchartWizardLabelsLegendPage.h"
#include "kchart_view.h"
#include "kchart_part.h"

#include <qbuttongroup.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qlabel.h>
#include <klocale.h>
#include <kfontdialog.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qcolor.h>

#include "kdchart/KDChartParams.h"

KChartWizardLabelsLegendPage::KChartWizardLabelsLegendPage( QWidget* parent, KChartPart* chart ) :
    QWidget( parent ),
    _chart( chart )
{
    ytitle2=true;

    KDChartAxisParams leftparams( _chart->params()->axisParams( KDChartAxisParams::AxisPosLeft ) );
    KDChartAxisParams bottomparams( _chart->params()->axisParams( KDChartAxisParams::AxisPosBottom ) );
    x_color=bottomparams.axisLineColor();
    y_color=leftparams.axisLineColor();
    xlabel=bottomparams.axisLabelsFont();
    ylabel=leftparams.axisLabelsFont();

    QGridLayout *grid1 = new QGridLayout(this,2,2,15,15);

    QGroupBox* tmpQGroupBox;
    tmpQGroupBox = new QGroupBox( this, "GroupBox_1" );
    tmpQGroupBox->setFrameStyle( 49 );

    QGridLayout *grid2 = new QGridLayout(tmpQGroupBox,4,4,15,7);

    QLabel* titleLA = new QLabel( i18n( "Title:" ),tmpQGroupBox );
    grid2->addWidget(titleLA,0,0);

    _titleED = new QLineEdit( tmpQGroupBox );
    grid2->addWidget(_titleED,0,1);
    _titleED->setText( _chart->params()->header1Text() );

    titlefont = new QPushButton( tmpQGroupBox );
    grid2->addWidget(titlefont,0,2);
    titlefont->setText(i18n("Font"));

    titlecolor=new KColorButton(tmpQGroupBox);
    grid2->addWidget(titlecolor,0,3);
    // PENDING(kalle) Put back in
    //   title_color=_chart->params()->TitleColor;
    //   titlecolor->setColor( title_color );


    QLabel* xlabelLA = new QLabel( i18n( "X-Title:" ), tmpQGroupBox );
    grid2->addWidget(xlabelLA,1,0);

    _xlabelED = new QLineEdit( tmpQGroupBox );
    // PENDING(kalle) Put back in
    //   _xlabelED->setText(_chart->params()->xtitle);
    grid2->addWidget(_xlabelED,1,1);

    xtitlefont = new QPushButton( tmpQGroupBox );
    xtitlefont->setText(i18n("Font"));
    grid2->addWidget(xtitlefont,1,2);

    xtitlecolor=new KColorButton(tmpQGroupBox);
    // PENDING(kalle) Put back in
    //  x_color=_chart->params()->XTitleColor;
    xtitlecolor->setColor( x_color );
    grid2->addWidget(xtitlecolor,1,3);


    QLabel* ylabelLA = new QLabel( i18n( "Y-Title:" ), tmpQGroupBox );
    grid2->addWidget(ylabelLA,2,0);

    _ylabelED = new QLineEdit( tmpQGroupBox );
    // PENDING(kalle) Put back in
    //   _ylabelED->setText(_chart->params()->ytitle);
    grid2->addWidget(_ylabelED,2,1);


    ytitlefont = new QPushButton( tmpQGroupBox);
    ytitlefont->setText(i18n("Font"));
    grid2->addWidget(ytitlefont,2,2);

    ytitlecolor=new KColorButton(tmpQGroupBox);
    // PENDING(kalle) Put back in
    //   y_color=_chart->params()->YTitleColor;
    ytitlecolor->setColor( y_color );
    grid2->addWidget(ytitlecolor,2,3);

    //ytitle2 doesn't work
    QLabel* ylabelLA2 = new QLabel( i18n( "Y-Title2:" ), tmpQGroupBox );
    grid2->addWidget(ylabelLA2,3,0);

    _ylabel2ED = new QLineEdit( tmpQGroupBox );
    // PENDING(kalle) Put back in
    //   _ylabel2ED->setText(_chart->params()->ytitle2);
    grid2->addWidget(_ylabel2ED,3,1);

    ytitle2color=new KColorButton(tmpQGroupBox);
    // PENDING(kalle) Put back in
    //   y_color2=_chart->params()->YTitle2Color;
    ytitle2color->setColor( y_color2 );
    grid2->addWidget(ytitle2color,3,2);

    // PENDING(kalle) Put back in
    //   xlabel=_chart->params()->xTitleFont();
    //   ylabel=_chart->params()->yTitleFont();

    title=_chart->params()->header1Font();
    grid1->addWidget(tmpQGroupBox,0,0);

    connect(xtitlefont,SIGNAL(clicked()),this,SLOT(changeXLabelFont()));
    connect(ytitlefont,SIGNAL(clicked()),this,SLOT(changeYLabelFont()));
    connect(titlefont,SIGNAL(clicked()),this,SLOT(changeTitleFont()));


    connect(xtitlecolor,SIGNAL(changed( const QColor & )),
            this,SLOT(changeXLabelColor(const QColor &)));
    connect(ytitlecolor,SIGNAL(changed( const QColor & )),
            this,SLOT(changeYLabelColor(const QColor &)));
    connect(titlecolor,SIGNAL(changed( const QColor & )),
            this,SLOT(changeTitleColor(const QColor &)));
    connect(ytitle2color,SIGNAL(changed( const QColor & )),
            this,SLOT(changeYTitle2Color(const QColor &)));

    resize( 600, 300 );
}


KChartWizardLabelsLegendPage::~KChartWizardLabelsLegendPage()
{
    //  _chart->removeAutoUpdate( preview );
}

void KChartWizardLabelsLegendPage::changeXLabelFont()
{
    if (KFontDialog::getFont( xlabel,false,this ) == QDialog::Rejected )
        return;

}
void KChartWizardLabelsLegendPage::changeYLabelFont()
{
    if (KFontDialog::getFont( ylabel ,false,this ) == QDialog::Rejected )
        return;

}
void KChartWizardLabelsLegendPage::changeTitleFont()
{
    if (KFontDialog::getFont( title ,false,this ) == QDialog::Rejected )
        return;

}

void KChartWizardLabelsLegendPage::changeXLabelColor(const QColor &_color)
{
    x_color=_color;
}

void KChartWizardLabelsLegendPage::changeYLabelColor(const QColor &_color)
{
    y_color=_color;
}

void KChartWizardLabelsLegendPage::changeTitleColor(const QColor &_color)
{
    title_color=_color;
}

void KChartWizardLabelsLegendPage::changeYTitle2Color(const QColor &_color)
{
    y_color2=_color;
}

void KChartWizardLabelsLegendPage::paintEvent( QPaintEvent * )
{
    if( ytitle2 ) {
        _ylabel2ED->setEnabled(true);
        ytitle2color->setEnabled(true);
    }
    else {
        _ylabel2ED->setEnabled(false);
        ytitle2color->setEnabled(false);
    }
}

void KChartWizardLabelsLegendPage::apply(  )
{
    _chart->params()->setHeader1Text( _titleED->text() );

    // PENDING(kalle) Put back in
    //    _chart->params()->setXTitleFont(xlabel);
    //    _chart->params()->setYTitleFont(ylabel);
    //    _chart->params()->xtitle= _xlabelED->text();
    //    _chart->params()->ytitle= _ylabelED->text();
    //    _chart->params()->XTitleColor=x_color;
    //    _chart->params()->YTitleColor=y_color;
    //    _chart->params()->TitleColor=title_color;
    //    _chart->params()->YTitle2Color=y_color2;
    //    _chart->params()->ytitle2=_ylabel2ED->text();
    _chart->params()->setHeader1Font(title);


    KDChartAxisParams leftparams = _chart->params()->axisParams( KDChartAxisParams::AxisPosLeft );
    KDChartAxisParams bottomparams = _chart->params()->axisParams( KDChartAxisParams::AxisPosBottom );
    if( x_color.isValid() )
        bottomparams.setAxisLineColor( x_color );
    if( y_color.isValid() )
        leftparams.setAxisLineColor( y_color );
    if(bottomparams.axisLabelsFont()!=xlabel)
        bottomparams.setAxisLabelsFont(xlabel,false);
    if(leftparams.axisLabelsFont()!=ylabel)
        leftparams.setAxisLabelsFont(ylabel,true);

    _chart->params()->setAxisParams( KDChartAxisParams::AxisPosBottom, bottomparams );
    _chart->params()->setAxisParams( KDChartAxisParams::AxisPosLeft, leftparams );
}


#include "kchartWizardLabelsLegendPage.moc"
