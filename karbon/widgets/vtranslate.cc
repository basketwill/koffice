/* This file is part of the KDE project
   Made by Tomislav Lukman (tomislav.lukman@ck.tel.hr)
   Copyright (C) 2002, The Karbon Developers
*/

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qstring.h>
#include <klocale.h>
#include <knuminput.h>

#include "vtranslate.h"

VTranslate::VTranslate( QWidget* parent, const char* name )
	: QWidget( parent, name )
{
	setCaption( i18n( "Translate" ) );

	QVBoxLayout *mainlayout = new QVBoxLayout(this, 7);
	mainlayout->addSpacing(5);

	QGridLayout *inputlayout = new QGridLayout(this, 5, 3);
	mainlayout->addLayout(inputlayout);
	m_labelX = new QLabel(i18n("X:"), this);
	inputlayout->addWidget(m_labelX, 0, 0);
	labely = new QLabel(i18n("Y:"), this);
	inputlayout->addWidget(labely, 1, 0);
	inputlayout->addColSpacing(1, 1);
	inputlayout->addColSpacing(3, 5);
	m_inputX = new KDoubleNumInput(0.00, this);
	m_inputX->setRange(-10000.00, 10000.00, 1.00, false); //range is just for example - for now :-)
	inputlayout->addWidget(m_inputX, 0, 2);
	m_inputY = new KDoubleNumInput(0.00, this);
	m_inputY->setRange(-10000.00, 10000.00, 1.00, false);
	inputlayout->addWidget(m_inputY, 1, 2);
	m_labelUnit1 = new QLabel("", this);
	inputlayout->addWidget(m_labelUnit1, 0, 4);
	m_labelUnit2 = new QLabel("", this);
	inputlayout->addWidget(m_labelUnit2, 1, 4);
	mainlayout->addSpacing(5);
	m_checkBoxPosition = new QCheckBox(i18n("Relative &position"), this);
	mainlayout->addWidget(m_checkBoxPosition);
	mainlayout->addSpacing(5);
	m_buttonDuplicate = new QPushButton(i18n("&Duplicate"), this);
	mainlayout->addWidget(m_buttonDuplicate);
	mainlayout->addSpacing(1);
	m_buttonApply = new QPushButton(i18n("&Apply"), this);
	mainlayout->addWidget(m_buttonApply);

	mainlayout->activate();

	setFixedSize(baseSize()); //Set the size tp fixed values
}

VTranslate::~VTranslate()
{
}

void VTranslate::setUnits( const QString& units )
{
	m_labelUnit1->setText( units );
	m_labelUnit2->setText( units );
}

#include "vtranslate.moc"

