/* This file is part of the KDE project
   Copyright (C) 2001 George Staikos <staikos@kde.org>

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

#include <kapp.h>
#include <config.h>
#include <qstring.h>
#include <kdebug.h>

#include "pwdprompt.h"

#include <qlayout.h>
#include <klineedit.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <kdialog.h>
#include <klocale.h>


PasswordPrompt::~PasswordPrompt() {
   QApplication::restoreOverrideCursor();
}


PasswordPrompt::PasswordPrompt(QWidget *parent, const char *name) 
: KDialog(parent, name, true) {
   _password = QString::null;

QGridLayout *grid = new QGridLayout(this, 3, 6, KDialog::marginHint(),
                                                KDialog::spacingHint());

   _pwd = new KLineEdit(this);
   _prompt = new QLabel(i18n("Enter the passphrase for the document:"), this);
   _ok = new QPushButton(i18n("&Ok"), this);
   _cancel = new QPushButton(i18n("&Cancel"), this);

   grid->addMultiCellWidget(_prompt, 0, 0, 0, 5);
   grid->addMultiCellWidget(_pwd, 1, 1, 0, 4);
   grid->addMultiCellWidget(_ok, 2, 2, 3, 3);
   grid->addMultiCellWidget(_cancel, 2, 2, 4, 4);

   grid->activate();

   _pwd->setFocus();

   QApplication::setOverrideCursor(Qt::arrowCursor);

   connect(_ok, SIGNAL(pressed()), this, SLOT(ok()));
   connect(_cancel, SIGNAL(pressed()), this, SLOT(cancel()));
   connect(_pwd, SIGNAL(returnPressed(const QString&)), this, SLOT(doOk(const QString&)));
}


void PasswordPrompt::doOk(const QString&) {
   ok();
}


void PasswordPrompt::ok() {
   _password = _ok->text();
   emit setPassword(_pwd->text());
   accept();
}


void PasswordPrompt::cancel() {
   emit setPassword(QString::null);
   delete this;
   reject();
}


QString& PasswordPrompt::getPassword() {
  return _password;
}



#include "pwdprompt.moc"

