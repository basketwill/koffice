/* This file is part of the KDE project
   Copyright (C) 2008 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>

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

#ifndef SCPARALLELSNAKESWIPEHORIZONTALSTRATEGY_H
#define SCPARALLELSNAKESWIPEHORIZONTALSTRATEGY_H

#include "../SCMatrixWipeStrategy.h"

class SCParallelSnakesWipeHorizontalStrategy : public SCMatrixWipeStrategy
{
public:
    SCParallelSnakesWipeHorizontalStrategy(bool reverseTop, bool reverseBottom, bool reverse);
    virtual ~SCParallelSnakesWipeHorizontalStrategy();
protected:
    virtual int squareIndex(int x, int y, int columns, int rows);
    virtual Direction squareDirection(int x, int y, int columns, int rows);
    virtual int maxIndex(int columns, int rows);
private:
    bool m_reverseTop;
    bool m_reverseBottom;
};

#endif // SCPARALLELSNAKESWIPEHORIZONTALSTRATEGY_H
