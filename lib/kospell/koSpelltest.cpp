/* This file is part of the KDE libraries
   Copyright (C) 2002 Laurent Montel <lmontel@mandrakesoft.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kapplication.h>
#include <kdebug.h>

#include <qstring.h>
#include <qlabel.h>
#include <qtextcodec.h>
#include "applicationspelltest.h"


int main(int argc, char *argv[])
{
    KApplication *app = new KApplication(argc, argv, "KSpellTest");

    ApplicationWindowSpell * mw = new ApplicationWindowSpell();
    app->setMainWidget(mw);
    mw->setCaption( "SpellTest" );
    mw->show();
    return app->exec();
}







