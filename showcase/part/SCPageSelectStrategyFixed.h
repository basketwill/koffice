/* This file is part of the KDE project
   Copyright (C) 2010 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2011 Thomas Zander <zander@kde.org>

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

#ifndef SCPAGESELECTSTRATEGYFIXED_H
#define SCPAGESELECTSTRATEGYFIXED_H

#include "SCPageSelectStrategyBase.h"

/**
 * Get a fixed page
 */
class SCPageSelectStrategyFixed : public SCPageSelectStrategyBase
{
public:
    SCPageSelectStrategyFixed(const KoPAPage *page);
    virtual ~SCPageSelectStrategyFixed();

    virtual const KoPAPage *page() const;

private:
    const KoPAPage *m_page;
};

#endif /* SCPAGESELECTSTRATEGYFIXED_H */
