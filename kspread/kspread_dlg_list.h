/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
             (C) 1999, 2000, 2001 Montel Laurent <lmontel@mandrakesoft.com>
             (C) 1999-2002 The KSpread Team
                           www.koffice.org/kspread

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

#ifndef __kspread_dlg_list__
#define __kspread_dlg_list__

#include <kdialogbase.h>

class QListBox;
class QListBoxItem;
class QLineEdit;
class QMultiLineEdit;
class QPushButton;

class KConfig;
class KSpreadView;

class KSpreadList: public KDialogBase
{
  Q_OBJECT

public:
  KSpreadList( KSpreadView* parent, const char* name );
  void init();

public slots:
  virtual void slotOk();
  void slotDoubleClicked(QListBoxItem *);
  void slotTextClicked(QListBoxItem*);
  void slotRemove();
  void slotAdd();
  void slotNew();
  void slotModify();
  void slotCopy();

protected:
  KConfig *config;

  QListBox * list;
  QMultiLineEdit *entryList;
  QPushButton* m_pOk;
  QPushButton* m_pCancel;
  QPushButton* m_pRemove;
  QPushButton* m_pNew;
  QPushButton* m_pAdd;
  QPushButton* m_pModify;
  QPushButton* m_pCopy;
  bool m_bChanged;
};

#endif
