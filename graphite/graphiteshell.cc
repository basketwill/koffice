/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2000 Werner Trobin <wtrobin@mandrakesoft.com>

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

#include <graphitepart.h>
#include <graphitefactory.h>
#include <graphiteshell.h>
#include <klocale.h>


GraphiteShell::GraphiteShell(const char *name)
                            : KoMainWindow(GraphiteFactory::global(), name) {
}

GraphiteShell::~GraphiteShell() {
}

QString GraphiteShell::nativeFormatName() const {
  return i18n("graphite");
}

KoDocument *GraphiteShell::createDoc() {
    return new GraphitePart;
}
#include <graphiteshell.moc>
