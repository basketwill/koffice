/* This file is part of the KDE project
   Copyright (C) 2003 Norbert Andres <nandres@web.de>
             (C) 2002 Ariya Hidayat <ariya@kde.org>
             (C) 2002 Philipp Mueller <philipp.mueller@gmx.de>
             (C) 1999-2002 Laurent Montel <montel@kde.org>
             (C) 1998-1999 Torben Weis <weis@kde.org>

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

#include <q3multilineedit.h>
#include <QPushButton>
#include <QLayout>
//Added by qt3to4:
#include <QVBoxLayout>

#include <klocale.h>
#include <kbuttonbox.h>

//KSpread includes
#include "Cell.h"
#include "Canvas.h"
#include "Doc.h"
#include "Selection.h"
#include "Sheet.h"
#include "View.h"

#include "CommentDialog.h"

using namespace KSpread;

CommentDialog::CommentDialog( View* parent, const char* name,const QPoint &_marker)
  : KDialog( parent )
{
    setCaption( i18n("Cell Comment") );
    setObjectName( name );
    setModal( true );
    setButtons( Ok|Cancel);

    m_pView = parent;
    marker= _marker;
    QWidget *page = new QWidget();
    setMainWidget( page );
    QVBoxLayout *lay1 = new QVBoxLayout( page );
    lay1->setMargin(KDialog::marginHint());
    lay1->setSpacing(KDialog::spacingHint());

    multiLine = new Q3MultiLineEdit( page );
    lay1->addWidget(multiLine);

    multiLine->setFocus();

    const QString comment = m_pView->activeSheet()->comment( marker.x(), marker.y() );
    if ( !comment.isEmpty() )
        multiLine->setText( comment );

    connect( this, SIGNAL( okClicked() ), this, SLOT( slotOk() ) );
    connect(multiLine, SIGNAL(textChanged ()),this, SLOT(slotTextChanged()));

    slotTextChanged();

    resize( 400, height() );
}

void CommentDialog::slotTextChanged()
{
    enableButtonOk( !multiLine->text().isEmpty());
}

void CommentDialog::slotOk()
{
    m_pView->setSelectionComment( multiLine->text() );
    accept();
}

#include "CommentDialog.moc"
