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

#include "kword_doc.h"
#include "kword_shell.h"

#include <koApplication.h>

#include <kregfactories.h>
#include <kregistry.h>

extern "C"
{
    void* init_kword();
}

int main( int argc, char **argv )
{
    KoApplication app( argc, argv );

    init_kword();
	
    KRegistry* registry = new KRegistry;
    registry->load();
    KRegistry::self()->addFactory( new KServiceTypeFactory );
    KRegistry::self()->addFactory( new KServiceFactory );

    KWordShell* shell = new KWordShell;

    KWordDocument* doc = new KWordDocument;
    doc->initDoc();
    shell->setRootPart( doc );

    shell->show();
    app.setMainWidget( shell );

    app.exec();

    return 0;
}
