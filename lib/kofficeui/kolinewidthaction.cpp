/* This file is part of the KDE project
   Copyright (C) 2004 Peter Simonsson <psn@linux.se>

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

#include "kolinewidthaction.h"

#include <qpainter.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qwhatsthis.h>
#include <qmenubar.h>
#include <qlayout.h>
#include <qlabel.h>

#include <kpopupmenu.h>
#include <kapplication.h>
#include <kdebug.h>
#include <ktoolbar.h>
#include <ktoolbarbutton.h>
#include <kiconloader.h>
#include <klocale.h>

#include <kozoomhandler.h>
#include <koUnitWidgets.h>
#include <koGlobal.h>

class KoLineWidthAction::KoLineWidthActionPrivate
{
  public:
    KoLineWidthActionPrivate()
    {
      m_currentIndex = 0;
      m_currentWidth = 1.0;
      m_unit = KoUnit::U_PT;
    }
    
    ~KoLineWidthActionPrivate()
    {
    }
    
    double m_currentWidth;
    int m_currentIndex;
    KoUnit::Unit m_unit;
};

KoLineWidthAction::KoLineWidthAction(const QString &text, const QString& icon,
  QObject* parent, const char* name) : KoSelectAction(text, icon, parent, name)
{
  d = new KoLineWidthActionPrivate;
  
  createMenu();
}

KoLineWidthAction::KoLineWidthAction(const QString &text, const QString& icon, const QObject* receiver,
  const char* slot, QObject* parent, const char* name) : KoSelectAction(text, icon, parent, name)
{
  d = new KoLineWidthActionPrivate;
  
  createMenu();
  
  connect(this, SIGNAL(newLineWidth(double)), receiver, slot);
}

KoLineWidthAction::~KoLineWidthAction()
{
  delete d;
}

void KoLineWidthAction::createMenu()
{
  KPopupMenu* popup = popupMenu();
  KoZoomHandler zoom;
  zoom.setZoomAndResolution(100, KoGlobal::dpiX(), KoGlobal::dpiY());
  QBitmap mask;
  QPixmap pix(70, 21);
  QPainter p(&pix, popup);
  int cindex = 0;
  QPen pen;

  for(int i = 1; i <= 10; i++) {
    pix.fill(white);
    pen.setWidth(zoom.zoomItY(i));
    p.setPen(pen);
    p.drawLine(0, 10, pix.width(), 10);
    mask = pix;
    pix.setMask(mask);
    popup->insertItem(pix,cindex++);
  }
  
  popup->insertSeparator(cindex++);
  popup->insertItem(i18n("&Custom..."), cindex++);
}

void KoLineWidthAction::execute(int index)
{
  bool ok = false;
  
  if((index >= 0) && (index < 10)) {
    d->m_currentWidth = (double) index + 1.0;
    ok = true;
  } if(index == 11) { // Custom width dialog...
    KoLineWidthChooser dlg;
    dlg.setUnit(d->m_unit);
    dlg.setWidth(d->m_currentWidth);
    
    if(dlg.exec()) {
      d->m_currentWidth = dlg.width();
      ok = true;
    }
  }

  if(ok) {
    popupMenu()->setItemChecked(d->m_currentIndex, false);
    popupMenu()->setItemChecked(index, true);
    d->m_currentIndex = index;
    emit newLineWidth(d->m_currentWidth);
  }
}

double KoLineWidthAction::currentWidth()
{
  return d->m_currentWidth;
}

void KoLineWidthAction::setCurrentWidth(double width)
{
  d->m_currentWidth = width;
  
  // Check if it is a standard width...
  for(int i = 1; i <= 10; i++) {
    if(KoUnit::toPoint(width) == (double) i) {
      popupMenu()->setItemChecked(d->m_currentIndex, false);
      popupMenu()->setItemChecked(i - 1, true);
      d->m_currentIndex = i - 1;
      return;
    }
  }

  //Custom width...
  popupMenu()->setItemChecked(d->m_currentIndex, false);
  popupMenu()->setItemChecked(11, true);
  d->m_currentIndex = 11;
}

void KoLineWidthAction::setUnit(KoUnit::Unit unit)
{
  d->m_unit = unit;
}

//////////////////////////////////////////////////
//
// KoLineWidthChooser
//

class KoLineWidthChooser::KoLineWidthChooserPrivate
{
  public:
    KoUnit::Unit m_unit;
    KoUnitDoubleSpinBox* m_lineWidthUSBox;
};

KoLineWidthChooser::KoLineWidthChooser(QWidget* parent, const char* name)
 : KDialogBase(parent, name, true, i18n("Custom Line Width"), Ok|Cancel, Ok)
{
  d = new KoLineWidthChooserPrivate;
  d->m_unit = KoUnit::U_PT;

  // Create the ui
  QWidget* mainWidget = new QWidget(this);
  setMainWidget(mainWidget);
  QGridLayout* gl = new QGridLayout(mainWidget, 1, 2, KDialog::marginHint(), KDialog::spacingHint());
  QLabel* textLbl = new QLabel(i18n("Line Width:"), mainWidget);
  d->m_lineWidthUSBox = new KoUnitDoubleSpinBox(mainWidget, 0.0, 1000.0, 0.1, 1.0, d->m_unit, 2);
  gl->addWidget(textLbl, 0, 0);
  gl->addWidget(d->m_lineWidthUSBox, 0, 1);
}

KoLineWidthChooser::~KoLineWidthChooser()
{
  delete d;
}

double KoLineWidthChooser::width()
{
  return KoUnit::fromUserValue(d->m_lineWidthUSBox->value(), d->m_unit);
}

void KoLineWidthChooser::setUnit(KoUnit::Unit unit)
{
  d->m_unit = unit;
  d->m_lineWidthUSBox->setUnit(unit);
}

void KoLineWidthChooser::setWidth(double width)
{
  d->m_lineWidthUSBox->changeValue(KoUnit::toUserValue(width, d->m_unit));
}

#include "kolinewidthaction.moc"
