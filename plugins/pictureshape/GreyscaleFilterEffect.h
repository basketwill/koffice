/* This file is part of the KDE project
 * Copyright (C) 2010 Carlos Licea <carlos@kdab.com>
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

#ifndef GREYSCALEFILTEREFFECT_H
#define GREYSCALEFILTEREFFECT_H

#include <KFilterEffect.h>

#define GreyscaleFilterEffectId "GreyscaleFilterEffectId"

class GreyscaleFilterEffect : public KFilterEffect
{
public:
    GreyscaleFilterEffect();
    virtual ~GreyscaleFilterEffect();
    virtual void save(KXmlWriter& writer);
    virtual bool load(const KXmlElement& element, const KFilterEffectLoadingContext& context);
    virtual QImage processImage(const QImage& image, const KFilterEffectRenderContext& context) const;
};

#endif // GRAYSCALEFILTEREFFECT_H
