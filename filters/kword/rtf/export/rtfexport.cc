// $Header$

/*
   This file is part of the KDE project
   Copyright 2001, 2002 Nicolas GOUTTE <goutte@kde.org>

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

#include <KWEFBaseWorker.h>
#include <KWEFKWordLeader.h>

#include "ExportFilter.h"

#include <rtfexport.h>
#include <rtfexport.moc>

typedef KGenericFactory<RTFExport, KoFilter> RTFExportFactory;
K_EXPORT_COMPONENT_FACTORY( librtfexport, RTFExportFactory( "kwordrtfexportfilter" ) )

//
// RTFExport
//

RTFExport::RTFExport(KoFilter *, const char *, const QStringList &) :
                     KoFilter() {
}

KoFilter::ConversionStatus RTFExport::convert( const QCString& from, const QCString& to )
{
    if ((from != "application/x-kword") || (to != "text/rtf"))
    {
        return KoFilter::NotImplemented;
    }

    KImageIO::registerFormats();

    RTFWorker* worker=new RTFWorker();

    KWEFKWordLeader* leader=new KWEFKWordLeader(worker);

    if (!leader)
    {
        kdError(30515) << "Cannot create Worker! Aborting!" << endl;
        delete worker;
        return KoFilter::StupidError;
    }
    
    KoFilter::ConversionStatus result=leader->convert(m_chain, from,to );

    delete leader;
    delete worker;

    return result;
}

