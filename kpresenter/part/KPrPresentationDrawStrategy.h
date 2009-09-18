/* This file is part of the KDE project
 * Copyright (C) 2009 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2009 Alexia Allanic <alexia_allanic@yahoo.fr>
 * Copyright (C) 2009 Jérémy Lugagne <jejewindsurf@hotmail.com>
 * Copyright (C) 2009 Thorsten Zachmann <zachmann@kde.org>
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
#ifndef KPRPRESENTATIONDRAWSTRATEGY_H
#define KPRPRESENTATIONDRAWSTRATEGY_H

#include "KPrPresentationStrategyBase.h"

class KPrPresentationDrawWidget;

class KPrPresentationDrawStrategy : public KPrPresentationStrategyBase
{
public:
    KPrPresentationDrawStrategy( KPrPresentationTool * tool );
    virtual ~KPrPresentationDrawStrategy();

    virtual bool keyPressEvent( QKeyEvent * event );

private:
    KPrPresentationDrawWidget * m_drawWidget;
};

#endif /* KPRPRESENTATIONDRAWSTRATEGY_H */

