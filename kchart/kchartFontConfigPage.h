/*
 * $Id$
 *
 * Copyright 2000 by Laurent Montel, released under Artistic License.
 */

#ifndef __KCHARTFONTCONFIGPAGE_H__
#define __KCHARTFONTCONFIGPAGE_H__

#include <qwidget.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qfont.h>
#include <kcolorbtn.h>
#include "kchartparams.h"

class KChartFontConfigPage : public QWidget
{
    Q_OBJECT

public:
    KChartFontConfigPage( KChartParameters* params,QWidget* parent );
    void init();
    void apply();
    void initList();

public slots:
    void changeIndex(int index);
    void changeLabelFont();
private:
    KChartParameters* _params;
    QLineEdit *font;
    QListBox *list;
    QListBox *listColor;
    QPushButton *fontButton;
    KColorButton *colorButton;
    QFont title;
    QFont xtitle;
    QFont ytitle;
    QFont label;
    QFont yaxis;
    QFont xaxis;
    KChartColorArray extColor;
    int indice;
};
#endif
