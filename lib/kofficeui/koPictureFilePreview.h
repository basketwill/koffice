/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

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

#ifndef kopicturefilepreview_h
#define kopicturefilepreview_h

#include <kpreviewwidgetbase.h>

class KoPictureFilePreviewWidget;

/**
 * A preview widget for KFileDialog that supports both pixmaps and cliparts.
 *
 * If fd is a KFileDialog *,
 *     fd->setPreviewWidget( new KoPictureFilePreview( fd ) );
 */
class KoPictureFilePreview : public KPreviewWidgetBase
{
    Q_OBJECT

public:
    KoPictureFilePreview( QWidget *parent );

public slots:
    virtual void showPreview(const KURL &url);
    virtual void clearPreview();

private:
    KoPictureFilePreviewWidget *m_widget;
    // m_widget can act as a d pointer for BC purposes, no need for another one
};

#endif





