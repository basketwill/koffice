/* This file is part of the KDE project
 * Copyright (c) 2009 Jan Hambrecht <jaham@gmx.net>
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

#ifndef BLUREFFECTCONFIGWIDGET_H
#define BLUREFFECTCONFIGWIDGET_H

#include "KFilterEffectConfigWidgetBase.h"

class KFilterEffect;
class BlurEffect;
class KDoubleNumInput;

class BlurEffectConfigWidget : public KFilterEffectConfigWidgetBase
{
    Q_OBJECT
public:
    BlurEffectConfigWidget(QWidget *parent = 0);

    /// reimplemented from KFilterEffectConfigWidgetBase
    virtual bool editFilterEffect(KFilterEffect * filterEffect);

private slots:
    void stdDeviationChanged(double stdDeviation);

private:
    BlurEffect * m_effect;
    KDoubleNumInput * m_stdDeviation;
};

#endif // BLUREFFECTCONFIGWIDGET_H
