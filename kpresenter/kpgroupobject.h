// -*- Mode: c++-mode; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4; -*-
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

#ifndef kpgroupobject_h
#define kpgroupobject_h

class KPresenterDoc;

#include "kpobject.h"

class KPGroupObject : public KPObject
{
public:
    KPGroupObject();
    KPGroupObject( const QPtrList<KPObject> &objs );
    virtual ~KPGroupObject() {}

    KPGroupObject &operator=( const KPGroupObject & );

    virtual void setSize( double _width, double _height );
    virtual void setOrig( const KoPoint &_point );
    virtual void setOrig( double _x, double _y );
    virtual void moveBy( const KoPoint &_point );
    virtual void moveBy( double _dx, double _dy );

    virtual void rotate( float _angle );
    virtual void setShadowDistance( int _distance );
    virtual void setShadowDirection( ShadowDirection _direction );
    virtual void setShadowColor( const QColor &_color );
    virtual void setEffect( Effect _effect );
    virtual void setEffect2( Effect2 _effect2 );
    virtual void setPresNum( int _presNum );
    virtual void setDisappear( bool b );
    virtual void setDisappearNum( int num );
    virtual void setEffect3( Effect3 _effect3);
    virtual void setAppearTimer( int _appearTimer );
    virtual void setDisappearTimer( int _disappearTimer );
    virtual void setAppearSoundEffect( bool b );
    virtual void setDisappearSoundEffect( bool b );
    virtual void setAppearSoundEffectFileName( const QString &_a_fileName );
    virtual void setDisappearSoundEffectFileName( const QString &_d_fileName );

    virtual void setShadowParameter(int _distance,ShadowDirection _direction,const QColor &_color);

    virtual ObjType getType() const
        { return OT_GROUP; }
    virtual QString getTypeString() const
        { return i18n("Group"); }

    virtual QDomDocumentFragment save( QDomDocument& doc, double offset );
    virtual double load(const QDomElement &element, KPresenterDoc *doc);

    virtual void draw( QPainter *_painter, KoZoomHandler *_zoomhandler,
                       SelectionMode selectionMode, bool drawContour = FALSE );

    void setUpdateObjects( bool b )
        { updateObjs = b; }

    QPtrList<KPObject> getObjects()
        { return objects; }
    const QPtrList<KPObject> &objectList() const
        { return objects; }
    virtual void addTextObjects( QPtrList<KoTextObject> & ) const;

    void selectAllObj();
    void deSelectAllObj();

    virtual void setOwnClipping( bool _ownClipping );
    virtual void setSubPresStep( int _subPresStep );
    virtual void doSpecificEffects( bool _specEffects, bool _onlyCurrStep = true );
    virtual void getAllObjectSelectedList(QPtrList<KPObject> &lst,bool force=false );

protected:
    void updateSizes( double fx, double fy );
    void updateCoords( double dx, double dy );

    QPtrList<KPObject> objects;
    bool updateObjs;

};

#endif
