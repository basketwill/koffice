/*
 *  Copyright (c) 2002 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "RgbU8ColorSpace.h"

#include <QColor>
#include <QDomElement>
#include <QImage>

#include <klocale.h>

#include <KoIntegerMaths.h>
#include <KoColorSpaceRegistry.h>
#include "compositeops/KoCompositeOps.h"
#include "compositeops/KoCompositeOpAdd.h"
#include "compositeops/KoCompositeOpSubtract.h"

#include "RgbU8CompositeOp.h"

#define downscale(quantum)  (quantum) //((unsigned char) ((quantum)/257UL))
#define upscale(value)  (value) // ((quint8) (257UL*(value)))

class KoRgbU8InvertColorTransformation : public KoColorTransformation
{

public:

    KoRgbU8InvertColorTransformation(const KoColorSpace* cs) : m_colorSpace(cs), m_psize(cs->pixelSize()) {
    }

    virtual void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const {
        while (nPixels--) {
            dst[0] = KoColorSpaceMathsTraits<quint8>::max - src[0];
            dst[1] = KoColorSpaceMathsTraits<quint8>::max - src[1];
            dst[2] = KoColorSpaceMathsTraits<quint8>::max - src[2];
            dst[3] = src[3];

            src += m_psize;
            dst += m_psize;
        }

    }

private:

    const KoColorSpace* m_colorSpace;
    quint32 m_psize;
};

RgbU8ColorSpace::RgbU8ColorSpace(KoColorProfile *p) :
        LcmsColorSpace<KoRgbU8Traits>(colorSpaceId(), i18n("RGB (8-bit integer/channel)"),  TYPE_BGRA_8, icSigRgbData, p)
{
    addChannel(new KoChannelInfo(i18n("Red"),   2, KoChannelInfo::COLOR, KoChannelInfo::UINT8, 1, QColor(255, 0, 0)));
    addChannel(new KoChannelInfo(i18n("Green"), 1, KoChannelInfo::COLOR, KoChannelInfo::UINT8, 1, QColor(0, 255, 0)));
    addChannel(new KoChannelInfo(i18n("Blue"),  0, KoChannelInfo::COLOR, KoChannelInfo::UINT8, 1, QColor(0, 0, 255)));
    addChannel(new KoChannelInfo(i18n("Alpha"), 3, KoChannelInfo::ALPHA, KoChannelInfo::UINT8));

    init();

    // ADD, ALPHA_DARKEN, BURN, DIVIDE, DODGE, ERASE, MULTIPLY, OVER, OVERLAY, SCREEN, SUBTRACT
    addStandardCompositeOps<KoRgbU8Traits>(this);

    addCompositeOp(new RgbU8CompositeOp(this, COMPOSITE_DARKEN,  i18n("Darken")));
    addCompositeOp(new RgbU8CompositeOp(this, COMPOSITE_LIGHTEN,  i18n("Lighten")));
    addCompositeOp(new RgbU8CompositeOp(this, COMPOSITE_HUE,  i18n("Hue")));
    addCompositeOp(new RgbU8CompositeOp(this, COMPOSITE_SATURATION,  i18n("Saturation")));
    addCompositeOp(new RgbU8CompositeOp(this, COMPOSITE_VALUE,  i18n("Value")));
    addCompositeOp(new RgbU8CompositeOp(this, COMPOSITE_COLOR,  i18n("Color")));
    addCompositeOp(new RgbU8CompositeOp(this, COMPOSITE_IN,  i18n("In")));
    addCompositeOp(new RgbU8CompositeOp(this, COMPOSITE_OUT,  i18n("Out")));
    addCompositeOp(new RgbU8CompositeOp(this, COMPOSITE_DIFF,  i18n("Diff")));
    addCompositeOp(new RgbU8CompositeOp(this, COMPOSITE_BUMPMAP,  i18n("Bumpmap")));
    addCompositeOp(new RgbU8CompositeOp(this, COMPOSITE_CLEAR,  i18n("Clear")));
    addCompositeOp(new RgbU8CompositeOp(this, COMPOSITE_DISSOLVE,  i18n("Dissolve")));

}


KoColorTransformation* RgbU8ColorSpace::createInvertTransformation() const
{
    return new KoRgbU8InvertColorTransformation(this);
}

QString RgbU8ColorSpace::colorSpaceId()
{
    return QString("RGBA");
}


KoColorSpace* RgbU8ColorSpace::clone() const
{
    return new RgbU8ColorSpace(profile()->clone());
}

void RgbU8ColorSpace::colorToXML(const quint8* pixel, QDomDocument& doc, QDomElement& colorElt) const
{
    const KoRgbU8Traits::Pixel* p = reinterpret_cast<const KoRgbU8Traits::Pixel*>(pixel);
    QDomElement labElt = doc.createElement("RGB");
    labElt.setAttribute("r", KoColorSpaceMaths< KoRgbU8Traits::channels_type, qreal>::scaleToA(p->red));
    labElt.setAttribute("g", KoColorSpaceMaths< KoRgbU8Traits::channels_type, qreal>::scaleToA(p->green));
    labElt.setAttribute("b", KoColorSpaceMaths< KoRgbU8Traits::channels_type, qreal>::scaleToA(p->blue));
    labElt.setAttribute("space", profile()->name());
    colorElt.appendChild(labElt);
}

void RgbU8ColorSpace::colorFromXML(quint8* pixel, const QDomElement& elt) const
{
    KoRgbU8Traits::Pixel* p = reinterpret_cast<KoRgbU8Traits::Pixel*>(pixel);
    p->red = KoColorSpaceMaths< qreal, KoRgbU8Traits::channels_type >::scaleToA(elt.attribute("r").toDouble());
    p->green = KoColorSpaceMaths< qreal, KoRgbU8Traits::channels_type >::scaleToA(elt.attribute("g").toDouble());
    p->blue = KoColorSpaceMaths< qreal, KoRgbU8Traits::channels_type >::scaleToA(elt.attribute("b").toDouble());
    p->alpha = KoColorSpaceMathsTraits<quint8>::max;
}

quint8 RgbU8ColorSpace::intensity8(const quint8 * src) const
{
    const KoRgbU8Traits::Pixel* p = reinterpret_cast<const KoRgbU8Traits::Pixel*>(src);
    return (quint8)(p->red * 0.30 + p->green * 0.59 + p->blue * 0.11);
}
