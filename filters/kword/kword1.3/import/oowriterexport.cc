//

/* This file is part of the KDE project
   Copyright (C) 2001, 2002, 2003, 2004 Nicolas GOUTTE <goutte@kde.org>

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

/*
   This file is based on the old file:
    /home/kde/koffice/filters/kword/ascii/asciiexport.cc

   The old file was copyrighted by
    Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
    Copyright (c) 2000 ID-PRO Deutschland GmbH. All rights reserved.
                       Contact: Wolf-Michael Bolle <Bolle@ID-PRO.de>

   The old file was licensed under the terms of the GNU Library General Public
   License version 2.
*/

#include <kdebug.h>
#include <kgenericfactory.h>
#include <kimageio.h>

#include <koFilterChain.h>

#include "ExportFilter.h"
#include "oowriterexport.h"

class OOWRITERExportFactory : KGenericFactory<OOWRITERExport, KoFilter>
{
public: // ### TODO: load correct .po file
    OOWRITERExportFactory(void) : KGenericFactory<OOWRITERExport, KoFilter> ("kwordkword1dot3import")
    {}
};

K_EXPORT_COMPONENT_FACTORY( libkwordkword1dot3import, OOWRITERExportFactory() )

OOWRITERExport::OOWRITERExport(KoFilter */*parent*/, const char */*name*/, const QStringList &) :
                     KoFilter() {
}

KoFilter::ConversionStatus OOWRITERExport::convert( const QCString& from, const QCString& to )
{
    if ( to != "application/vnd.sun.xml.writer"  // ### TODO: OASIS
        || from != "application/x-kword" )
    {
        return KoFilter::NotImplemented;
    }

    // We need KimageIO's help in OOWriterWorker::convertUnknownImage
    KImageIO::registerFormats();

    OOWriterWorker* worker=new OOWriterWorker();

    if (!worker)
    {
        kdError(30506) << "Cannot create Worker! Aborting!" << endl;
        return KoFilter::StupidError;
    }

    KoFilter::ConversionStatus result=worker->convert(m_chain,from,to);

    delete worker;

    return result;
}

#include "oowriterexport.moc"
