/* This file is part of the KDE project
   Copyright (C) 2008 Sven Langkamp <sven.langkamp@gmail.com>

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

#ifndef SCCENTERFANWIPESTRATEGY_H
#define SCCENTERFANWIPESTRATEGY_H

#include <SCPageEffectStrategy.h>

class SCCenterFanWipeStrategy : public SCPageEffectStrategy
{
public:
    SCCenterFanWipeStrategy(int startAngle, int fanCount, int subType, const char * smilType, const char *smilSubType, bool reverse);
    virtual ~SCCenterFanWipeStrategy();

    virtual void setup(const SCPageEffect::Data &data, QTimeLine &timeLine);
    virtual void paintStep(QPainter &p, int currPos, const SCPageEffect::Data &data);
    virtual void next(const SCPageEffect::Data &data);

private:
    double m_startAngle;
    int m_fanCount;
};

#endif // SCCENTERFANWIPESTRATEGY_H
