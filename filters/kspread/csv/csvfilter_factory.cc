/* This file is part of the KDE project
   Copyright (C) 1999 David Faure <faure@kde.org>

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

#include "csvfilter_factory.h"
#include "csvfilter_factory.moc"
#include "csvfilter.h"

#include <kinstance.h>

extern "C"
{
    void* init_libcsvfilter()
    {
        return new CSVFilterFactory;
    }
};

KInstance* CSVFilterFactory::s_global = 0;

CSVFilterFactory::CSVFilterFactory( QObject* parent, const char* name )
    : KLibFactory( parent, name )
{
    s_global = new KInstance( "csvfilter" );
}

CSVFilterFactory::~CSVFilterFactory()
{
    delete s_global;
}

QObject* CSVFilterFactory::create( QObject* parent, const char* name, const char*, const QStringList & )
{
    if ( parent && !parent->inherits("KoFilter") )
    {
	    qDebug("CSVFilterFactory: parent does not inherit KoFilter");
	    return 0L;
    }
    CSVFilter *f = new CSVFilter( (KoFilter*)parent, name );
    emit objectCreated(f);
    return f;
}

KInstance* CSVFilterFactory::global()
{
    return s_global;
}
