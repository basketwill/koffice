/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <klocale.h>

#include <qlabel.h>
#include <qlayout.h>

#include "preferencesdlg.h"

/*****************************************************************************
 *
 * KImagePreferencesDialog
 *
 *****************************************************************************/

KImagePreferencesDialog::KImagePreferencesDialog(QWidget *parent, const char *name, const char *inputtitle, WFlags f )
	: KDialog( parent, name, true, f )
{
	// Layout
    QGridLayout* grid = new QGridLayout( this, 5, 5, 7, 15);

	// Inputline
	m_pLineEdit = new QLineEdit( this, i18n( "Directory for temporary files" ) );
    grid->addWidget( m_pLineEdit, 0, 0 );	
	// Label
	QLabel* label = new QLabel( m_pLineEdit, inputtitle, this );

	// OK-Button

	// Cancel-Button
}

KImagePreferencesDialog::~KImagePreferencesDialog()
{
}

QString KImagePreferencesDialog::getStr()
{
	QString tmp;

	return tmp;
}

#include "preferencesdlg.moc"
