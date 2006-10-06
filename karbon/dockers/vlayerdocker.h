/* This file is part of the KDE project
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef V_LAYER_DOCKER
#define V_LAYER_DOCKER

#include <QWidget>
#include <KoDocumentSectionModel.h>

class VDocument;
class KoDocumentSectionView;
class KoShape;
class KoLayerShape;
class QAbstractItemModel;
class VDocumentModel;

class VLayerDocker : public QWidget
{
Q_OBJECT

public:
    VLayerDocker( QWidget *parent, VDocument *doc );
    virtual ~VLayerDocker();
public slots:
    void updateView();
private slots:
    void slotButtonClicked( int buttonId );
    void addLayer();
private:
    VDocument *m_document;
    KoDocumentSectionView *m_layerView;
    VDocumentModel *m_model;
};

class VDocumentModel : public KoDocumentSectionModel
{
public:
    VDocumentModel( VDocument *doc );
    void update();
    // from QAbstractItemModel
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent( const QModelIndex &child ) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    virtual bool hasChildren ( const QModelIndex & parent = QModelIndex() ) const;
private:
    PropertyList properties( KoShape* shape ) const;
    void setProperties( KoShape* shape, const PropertyList &properties );
    VDocument *m_document;
    KoShape *m_shape;
};

#endif // V_LAYER_DOCKER

// kate: replace-tabs on; space-indent on; indent-width 4; mixedindent off; indent-mode cstyle;
