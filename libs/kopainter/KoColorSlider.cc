/* This file is part of the KDE project
   Copyright (C) 2006 Sven Langkamp <longamp@reallygood.de>

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

#include <QColor>
#include <QPainter>

#include "KoColorSlider.h"

KoColorSlider::KoColorSlider(KoColorSpace* colorSpace, QWidget* parent)
  : KSelector(parent)
  , m_colorSpace(colorSpace)
{
  color1 = KoColor( Qt::black, m_colorSpace);
  color2 = KoColor( Qt::white, m_colorSpace);
}

KoColorSlider::~KoColorSlider()
{
}

void KoColorSlider::setLeftColor(const KoColor& c)
{
  color1 = c;
  color1.convertTo(m_colorSpace);
  update();
}

void KoColorSlider::setRightColor(const KoColor& c)
{
  color2 = c;
  color2.convertTo(m_colorSpace);
  update();
}

void KoColorSlider::setColors( const KoColor& leftColor, const KoColor& rightColor)
{
  color1 = leftColor;
  color2 = rightColor;

  color1.convertTo(m_colorSpace);
  color2.convertTo(m_colorSpace);

  update();
}

void KoColorSlider::drawContents( QPainter *painter )
{
  QPixmap checker(8, 8);
  QPainter p(&checker);
  p.fillRect(0, 0, 4, 4, Qt::lightGray);
  p.fillRect(4, 0, 4, 4, Qt::darkGray);
  p.fillRect(0, 4, 4, 4, Qt::darkGray);
  p.fillRect(4, 4, 4, 4, Qt::lightGray);
  p.end();
  painter->fillRect(contentsRect(), QBrush(checker));

  KoColor c = color1;
  QColor color;
  quint8 opacity;

  const quint8 *colors[2];
  colors[0] = color1.data();
  colors[1] = color2.data();

  QImage image(contentsRect().width(), contentsRect().height(), QImage::Format_ARGB32 );

  for (int x = 0; x < contentsRect().width(); x++) {

      double t = static_cast<double>(x) / (contentsRect().width() - 1);

      quint8 colorWeights[2];
      colorWeights[0] = static_cast<quint8>((1.0 - t) * 255 + 0.5);
      colorWeights[1] = 255 - colorWeights[0];

      m_colorSpace->mixColors(colors, colorWeights, 2, c.data());

      c.toQColor( &color, &opacity );
      color.setAlpha(opacity);

      for (int y = 0; y < contentsRect().height(); y++)
        image.setPixel(x, y, color.rgba());
  }
  painter->drawImage( contentsRect(), image, QRect( 0, 0, image.width(), image.height()) );
}

#include "KoColorSlider.moc"
