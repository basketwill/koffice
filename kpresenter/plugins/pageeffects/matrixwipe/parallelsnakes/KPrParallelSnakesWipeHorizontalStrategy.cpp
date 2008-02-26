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

#include "KPrParallelSnakesWipeHorizontalStrategy.h"

static KPrPageEffect::SubType getSubType(bool reverseTop, bool reverseBottom, bool reverse) {
    if (reverseTop) {
        if (reverseBottom) return reverse ? KPrPageEffect::HorizontalRightSameOut : KPrPageEffect::HorizontalRightSameIn;
        else return reverse ? KPrPageEffect::HorizontalTopRightOppositeOut : KPrPageEffect::HorizontalTopRightOppositeIn;
    } else {
        if (reverseBottom) return reverse ? KPrPageEffect::HorizontalTopLeftOppositeOut : KPrPageEffect::HorizontalTopLeftOppositeIn;
        else return reverse ? KPrPageEffect::HorizontalLeftSameOut : KPrPageEffect::HorizontalLeftSameIn;
    }
}

static const char* getSmilSubType(bool reverseTop, bool reverseBottom) {
    if (reverseTop) {
        if (reverseBottom) return "horizontalRightSame";
        else return "horizontalTopRightOpposite";
    } else {
        if (reverseBottom) return "horizontalTopLeftOpposite";
        else return "horizontalLeftSame";
    }
}

KPrParallelSnakesWipeHorizontalStrategy::KPrParallelSnakesWipeHorizontalStrategy(bool reverseTop, bool reverseBottom, bool reverse)
    : KPrMatrixWipeStrategy( getSubType(reverseTop, reverseBottom, reverse), "parallelSnakesWipe", getSmilSubType(reverseTop, reverseBottom), reverse, true ),
    m_reverseTop(reverseTop), m_reverseBottom(reverseBottom)
{
    setNeedEvenSquares();
}

KPrParallelSnakesWipeHorizontalStrategy::~KPrParallelSnakesWipeHorizontalStrategy()
{
}

int KPrParallelSnakesWipeHorizontalStrategy::squareIndex(int x, int y, int collumns, int rows)
{
    int X = x;
    int idx;
    if (y < rows / 2) {
        if (m_reverseTop) X = collumns - X - 1;
        if (y & 1) X = collumns - X - 1;
     idx = X + y * collumns;
    } else {
        if (m_reverseBottom) X = collumns - X - 1;
        if (!(y & 1)) X = collumns - X - 1;
        idx = X + (rows - y - 1) * collumns;
    }
    if (reverse()) {
        return rows * collumns / 2 - idx - 1;
    } else {
        return idx;
    }
}

KPrMatrixWipeStrategy::Direction KPrParallelSnakesWipeHorizontalStrategy::squareDirection(int x, int y, int collumns, int rows)
{
    bool reverse = false;
    if (y >= rows / 2) reverse = !reverse;
    if (y & 1) reverse = !reverse;
    if (this->reverse()) reverse = !reverse;
    if (y < rows / 2) {
        if (m_reverseTop) reverse = !reverse;
    } else {
        if (m_reverseBottom) reverse = !reverse;
    }
    return reverse ? RightToLeft : LeftToRight;
}

int KPrParallelSnakesWipeHorizontalStrategy::maxIndex(int collumns, int rows)
{
    return collumns * rows / 2;
}

