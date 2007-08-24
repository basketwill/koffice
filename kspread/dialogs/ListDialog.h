/* This file is part of the KDE project
   Copyright (C) 2002-2003 Ariya Hidayat <ariya@kde.org>
             (C) 2001-2003 Laurent Montel <montel@kde.org>
             (C) 1998, 1999 Torben Weis <weis@kde.org>

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
   Boston, MA 02110-1301, USA.
*/

#ifndef KSPREAD_LIST_DIALOG
#define KSPREAD_LIST_DIALOG

#include <kdialog.h>
#include <kconfig.h>
#include <ksharedconfig.h>

class Q3ListBox;
class Q3MultiLineEdit;
class QPushButton;

namespace KSpread
{

class ListDialog: public KDialog
{
  Q_OBJECT

public:
  ListDialog( QWidget* parent, const char* name );
  void init();

public slots:
  virtual void slotOk();
  void slotDoubleClicked();
  void slotTextClicked();
  void slotAdd();
  void slotCancel();
  void slotNew();
  void slotRemove();
  void slotModify();
  void slotCopy();

protected:
  KSharedConfigPtr config;

  Q3ListBox * list;
  Q3MultiLineEdit *entryList;
  QPushButton* m_pAdd;
  QPushButton* m_pCancel;
  QPushButton* m_pRemove;
  QPushButton* m_pNew;
  QPushButton* m_pModify;
  QPushButton* m_pCopy;
  bool m_bChanged;
};

} // namespace KSpread

#endif // KSPREAD_LIST_DIALOG
