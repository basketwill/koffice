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

#include <config.h>

#include <qtextstream.h>
#include <kocryptimport.h>
#include <kdebug.h>



KoCryptImport::KoCryptImport(KoFilter *parent, const char *name) :
                             KoFilter(parent, name) {
}


bool KoCryptImport::filter(const QString &fileIn, const QString &fileOut,
                           const QString& from, const QString& to,
                           const QString &) {


    if(to != "application/x-kword" || from != "application/x-kword-crypt")
        return false;

    return true;
}



#include "kocryptimport.moc"

