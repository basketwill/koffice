/* This file is part of the KDE project
   Copyright (C) 2005 Laurent Montel <montel@kde.org>

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
 * Boston, MA 02110-1301, USA.
*/

#include <QPixmap>
#include <QPainter>

#include <kmessagebox.h>

#include <KoFilterChain.h>
#include <KoStore.h>
#include <kgenericfactory.h>
#include <KoDocument.h>

#include "bmpexport.h"
#include "exportsizedia.h"

typedef KGenericFactory<BmpExport> bmpExportFactory;
K_EXPORT_COMPONENT_FACTORY( libkchartbmpexport, bmpExportFactory( "bmpexport" ) )

BmpExport::BmpExport(QObject* parent, const QStringList&lst)
    : ImageExport(parent,lst)
{
}

BmpExport::~BmpExport()
{
}

void BmpExport::extraImageAttribute()
{
    ExportSizeDia  *exportDialog = new ExportSizeDia( width, height,
						   0);
    if (exportDialog->exec()) {
	width  = exportDialog->width();
	height = exportDialog->height();

	kDebug() << "PNG Export: size = [" << width << "," << height << "]" << endl;
    }
    delete exportDialog;
}


bool BmpExport::saveImage( const QString& fileName)
{
    bool ret = pixmap.save( fileName, "BMP" );
    // Save the image.
    if ( !ret ) {
        KMessageBox::error( 0, i18n( "Failed to write file." ),
                            i18n( "BMP Export Error" ) );
    }
    return ret;
}

const char * BmpExport::exportFormat()
{
	        return "image/x-bmp";
}

#include "bmpexport.moc"

