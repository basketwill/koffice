/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>
		 2006 Martin Pfeiffer <hubipete@gmx.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/


#include "KFormulaPartView.h"
#include "KFormulaPartDocument.h"
#include "KFormulaPartViewAdaptor.h"
#include "KFormulaPartFactory.h"
#include "KFormulaConfigDialog.h"
#include "KFormulaCanvas.h"

#include <QScrollArea>
#include <kstdaction.h>
#include <kaction.h>
#include <ktip.h>
#include <klocale.h>

#include <kglobal.h>
#include <kstandarddirs.h>

KFormulaPartView::KFormulaPartView( KFormulaPartDocument* doc, QWidget* parent,
                                                               const char* name )
        : KoView( doc, parent, name )
{
    m_partDocument = doc;
    m_formulaCanvas = new KFormulaCanvas( this );
	
    m_dbus = new KFormulaPartViewAdaptor( this );
    QDBus::sessionBus().registerObject( "/" + objectName(), this );

    setInstance( KFormulaPartFactory::global() );

    m_scrollArea = new QScrollArea( this );
    m_scrollArea->setWidget( m_formulaCanvas );

    if ( !doc->isReadWrite() )
        setXMLFile("kformula_readonly.rc");
    else
        setXMLFile("kformula.rc");

    KStdAction::tipOfDay( this, SLOT( slotShowTip() ), actionCollection() );
}

KFormulaPartView::~KFormulaPartView()
{
}

KFormulaPartViewAdaptor* KFormulaPartView::dbusObject()
{
    return m_dbus;
}

void KFormulaPartView::setupActions()
{
    KActionCollection* c = actionCollection();
    KGlobal::dirs()->addResourceType( "toolbar", KStandardDirs::kde_default("data") +
                                      "kformula/pics/" );
	
/*    m_cutAction;
    m_copyAction;
    m_pasteAction;*/

    m_addBracketAction = new KAction( i18n("Add Bracket"), c, "addbracket" );
    m_addFractionAction = new KAction( i18n("Add Fraction"), c, "addfraction" );
    m_addRootAction = new KAction( i18n("Add Root"), c, "addroot" );
    m_addSumAction = new KAction( i18n("Add Sum"), c, "addsum" );
    m_addProductAction = new KAction( i18n("Add Product"), c, "addproduct" );
    m_addIntegralAction = new KAction( i18n("Add Integral"), c, "addintegral" );
    m_addMatrixAction = new KAction( i18n("Add Matrix"), c, "addmatrix" );
/*    m_addUpperLeftAction;
    m_addLowerLeftAction;
    m_addUpperRightAction;
    m_addLowerRightAction;
    m_addGenericUpperAction;
    m_addGenericLowerAction;
    m_removeEnclosingAction;*/

//    KAction* m_formulaStringAction;
}

void KFormulaPartView::focusInEvent( QFocusEvent* )
{
    m_formulaCanvas->setFocus();
}

void KFormulaPartView::slotShowTipOnStart()
{
    KTipDialog::showTip( this );
}

void KFormulaPartView::slotShowTip()
{
    KTipDialog::showTip( this, QString::null, true );
}

void KFormulaPartView::setEnabled( bool enabled )
{
    m_addBracketAction->setEnabled( enabled );
    m_addFractionAction->setEnabled( enabled );
    m_addRootAction->setEnabled( enabled );
    m_addSumAction->setEnabled( enabled );
    m_addIntegralAction->setEnabled( enabled);
    m_addMatrixAction->setEnabled( enabled);
/*    m_addUpperLeftAction->setEnabled( enabled );
    m_addLowerLeftAction->setEnabled( enabled );
    m_addUpperRightAction->setEnabled( enabled );
    m_addLowerRightAction->setEnabled( enabled );
    m_addGenericUpperAction->setEnabled( enabled );
    m_addGenericLowerAction->setEnabled( enabled );
    m_removeEnclosingAction->setEnabled( enabled );*/
}

void KFormulaPartView::resizeEvent( QResizeEvent * )
{
    m_scrollArea->setGeometry( 0, 0, width(), height() );
}

void KFormulaPartView::setupPrinter( KPrinter& )
{
}

void KFormulaPartView::print( KPrinter& printer )
{

}

void KFormulaPartView::cursorChanged( bool visible, bool selecting )
{
    m_cutAction->setEnabled( visible && selecting );
    m_copyAction->setEnabled( visible && selecting );
    m_removeEnclosingAction->setEnabled( !selecting );
}

void KFormulaPartView::sizeSelected( int size )
{
//    document()->getFormula()->setFontSize( size );
    m_formulaCanvas->setFocus();
}

void KFormulaPartView::configure()
{
    KFormulaConfigDialog configDialog( this );
    configDialog.exec();
}

void KFormulaPartView::updateReadWrite( bool )
{
}

#include "KFormulaPartView.moc"
