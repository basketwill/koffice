/* This file is part of the KDE project
   Copyright (c) 2001 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2002 Nicolas GOUTTE <nicog@snafu.de>

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

#include <qbuffer.h>
#include <qpainter.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qapplication.h>

#include <kdebug.h>
#include <kdebugclasses.h>

#include "koPictureKey.h"
#include "koPictureBase.h"
#include "koPictureEps.h"


KoPictureEps::KoPictureEps(void) : m_cacheIsInFastMode(true)
{
    // Forbid QPixmap to cache the X-Window resources (Yes, it is slower!)
    m_cachedPixmap.setOptimization(QPixmap::MemoryOptim);
}

KoPictureEps::~KoPictureEps(void)
{
}

KoPictureBase* KoPictureEps::newCopy(void) const
{
    return new KoPictureEps(*this);
}

KoPictureType::Type KoPictureEps::getType(void) const
{
    return KoPictureType::TypeEps;
}

bool KoPictureEps::isNull(void) const
{
    return m_rawData.isNull();
}

void KoPictureEps::scaleAndCreatePixmap(const QSize& size, bool fastMode)
{
    kdDebug(30003) << "KoPictureEps::scaleAndCreatePixmap " << size << " " << (fastMode?QString("fast"):QString("slow")) << endl;
    if ((size==m_cachedSize)
        && ((fastMode) || (!m_cacheIsInFastMode)))
    {
        // The cached pixmap has already the right size
        // and:
        // - we are in fast mode (We do not care if the re-size was done slowly previously)
        // - the re-size was already done in slow mode
        kdDebug(30003) << "Already cached!" << endl;
        return;
    }

    // Slow mode can be very slow, especially at high zoom levels -> configurable
    if ( !isSlowResizeModeAllowed() )
    {
        kdDebug(30003) << "User has disallowed slow mode!" << endl;
        fastMode = true;
    }

    // We cannot use fast mode, if nothing was ever cached.
    if ( fastMode && !m_cachedSize.isEmpty())
    {
        kdDebug(30003) << "Fast scaling!" << endl;
#if 1
        // Slower than caching a QImage, but faster than re-sampling!
        QImage image( m_cachedPixmap.convertToImage() );
        m_cachedPixmap=image.scale( size );
#else
        // Very fast, but truncates or adds white!
        m_cachedPixmap.resize( size ); // Only resize, do not scale!
#endif
        m_cacheIsInFastMode=true;
    }
    else
    {
        kdDebug(30003) << "Re-sample!" << endl;
        QApplication::setOverrideCursor( Qt::waitCursor );
        QBuffer buffer( m_rawData );
        buffer.open( IO_ReadOnly );
        QImageIO io( &buffer, 0 );
        QCString params;
        params.setNum( size.width() );
        params += ':';
        QCString height;
        height.setNum( size.height() );
        params += height;
        io.setParameters( params );
        io.read();
        QImage image ( io.image() );
        if ( image.size() != size ) // this can happen due to rounding problems
        {
            //kdDebug() << "fixing size to " << size.width() << "x" << size.height()
            //          << " (was " << image.width() << "x" << image.height() << ")" << endl;
            image = image.scale( size ); // hmm, smoothScale instead?
        }
        m_cachedPixmap = image;
        QApplication::restoreOverrideCursor();
        m_cacheIsInFastMode=false;
    }
    m_cachedSize=size;
    kdDebug(30003) << "New size: " << size << endl;
}

void KoPictureEps::draw(QPainter& painter, int x, int y, int width, int height, int sx, int sy, int sw, int sh, bool fastMode)
{
    //kdDebug() << "KoImage::draw currentSize:" << currentSize.width() << "x" << currentSize.height() << endl;
    if ( !width || !height )
        return;

    QSize screenSize( width, height );
    //kdDebug() << "KoPictureEps::draw screenSize=" << screenSize.width() << "x" << screenSize.height() << endl;

    scaleAndCreatePixmap(screenSize, fastMode && !painter.device()->isExtDev() );

    // sx,sy,sw,sh is meant to be used as a cliprect on the pixmap, but drawPixmap
    // translates it to the (x,y) point -> we need (x+sx, y+sy).
    painter.drawPixmap( x + sx, y + sy, m_cachedPixmap, sx, sy, sw, sh );
}

bool KoPictureEps::load(QIODevice* io, const QString& /*extension*/)
{
    kdDebug(30003) << "KoPictureEps::load" << endl;
    // First, read the raw data
    m_rawData=io->readAll();

    QTextStream stream(m_rawData, IO_ReadOnly);
    QString lineBox;
    QRect rect;
    for(;;)
    {
        QString line( stream.readLine() );
        kdDebug(30003) << "Checking line: " << line << endl;
        if (line.startsWith("%%BoundingBox:"))
        {
            lineBox=line;
            break;
        }
        else if (line.startsWith("%%"))
            continue;
        else if (line.startsWith("%!"))
            continue;
        else
            break;
    }
    if (lineBox.isEmpty())
    {
        kdError(30003) << "KoPictureEps::load: could not find bounding box!" << endl;
        return false;
    }
    QRegExp exp("([0-9]+\\.?[0-9]*)\\s([0-9]+\\.?[0-9]*)\\s([0-9]+\\.?[0-9]*)\\s([0-9]+\\.?[0-9]*)");
    exp.search(lineBox);
    kdDebug(30003) << "Reg. Exp. Found: " << exp.capturedTexts() << endl;
    rect.setLeft(exp.cap(1).toDouble());
    rect.setTop(exp.cap(2).toDouble());
    rect.setRight(exp.cap(3).toDouble());
    rect.setBottom(exp.cap(4).toDouble());
    m_originalSize=rect.size();
    kdDebug(30003) << "Rect: " << rect << " Size: "  << m_originalSize << endl;
    return true;
}

bool KoPictureEps::save(QIODevice* io)
{
    // We save the raw data, to avoid damaging the file by many load/save cyvles (especially for JPEG)
    Q_ULONG size=io->writeBlock(m_rawData); // WARNING: writeBlock returns Q_LONG but size() Q_ULONG!
    return (size==m_rawData.size());
}

QSize KoPictureEps::getOriginalSize(void) const
{
    return m_originalSize;
}

QPixmap KoPictureEps::generatePixmap(const QSize& size, bool smoothScale)
{
    scaleAndCreatePixmap(size,!smoothScale);
    return m_cachedPixmap;
}

QString KoPictureEps::getMimeType(const QString&) const
{
    return "image/x-eps";
}
