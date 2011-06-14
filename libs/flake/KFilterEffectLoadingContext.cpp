/* This file is part of the KDE project
* Copyright (c) 2010 Jan Hambrecht <jaham@gmx.net>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this library; see the file COPYING.LIB.  If not, write to
* the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA 02110-1301, USA.
*/

#include "KFilterEffectLoadingContext.h"

#include <QtCore/QString>
#include <QtCore/QRectF>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>

class KFilterEffectLoadingContext::Private
{
public:
    Private()
        : convertFilterUnits(false), convertFilterPrimitiveUnits(false)
    {}
    QString basePath;
    QRectF shapeBound;
    bool convertFilterUnits;
    bool convertFilterPrimitiveUnits;
};

KFilterEffectLoadingContext::KFilterEffectLoadingContext(const QString &basePath)
    : d(new Private())
{
    d->basePath = basePath;
}

KFilterEffectLoadingContext::~KFilterEffectLoadingContext()
{
    delete d;
}

void KFilterEffectLoadingContext::setShapeBoundingBox(const QRectF &shapeBound)
{
    d->shapeBound = shapeBound;
}

void KFilterEffectLoadingContext::enableFilterUnitsConversion(bool enable)
{
    d->convertFilterUnits = enable;
}

void KFilterEffectLoadingContext::enableFilterPrimitiveUnitsConversion(bool enable)
{
    d->convertFilterPrimitiveUnits = enable;
}

QPointF KFilterEffectLoadingContext::convertFilterUnits(const QPointF &value) const
{
    if (!d->convertFilterUnits)
        return value;

    return QPointF(convertFilterUnitsX(value.x()), convertFilterUnitsY(value.y()));
}

qreal KFilterEffectLoadingContext::convertFilterUnitsX(qreal value) const
{
    if (!d->convertFilterUnits)
        return value;

    return value / d->shapeBound.width();
}

qreal KFilterEffectLoadingContext::convertFilterUnitsY(qreal value) const
{
    if (!d->convertFilterUnits)
        return value;

    return value / d->shapeBound.height();
}

QPointF KFilterEffectLoadingContext::convertFilterPrimitiveUnits(const QPointF &value) const
{
    if (!d->convertFilterPrimitiveUnits)
        return value;

    return QPointF(convertFilterPrimitiveUnitsX(value.x()), convertFilterPrimitiveUnitsY(value.y()));
}

qreal KFilterEffectLoadingContext::convertFilterPrimitiveUnitsX(qreal value) const
{
    if (!d->convertFilterPrimitiveUnits)
        return value;

    return value / d->shapeBound.width();
}

qreal KFilterEffectLoadingContext::convertFilterPrimitiveUnitsY(qreal value) const
{
    if (!d->convertFilterPrimitiveUnits)
        return value;

    return value / d->shapeBound.height();
}

QString KFilterEffectLoadingContext::pathFromHref(const QString &href) const
{
    QFileInfo info(href);
    if (! info.isRelative())
        return href;

    QFileInfo pathInfo(QFileInfo(d->basePath).filePath());

    QString relFile = href;
    while (relFile.startsWith("../")) {
        relFile = relFile.mid(3);
        pathInfo.setFile(pathInfo.dir(), QString());
    }

    QString absFile = pathInfo.absolutePath() + '/' + relFile;

    return absFile;
}