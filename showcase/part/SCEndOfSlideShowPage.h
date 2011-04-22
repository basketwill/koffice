/* This file is part of the KDE project
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KPRENDOFSLIDESHOWPAGE_H
#define KPRENDOFSLIDESHOWPAGE_H

class SCDocument;

#include "SCPage.h"

/**
 * This is the page that is use as end of presentation slide show.
 *
 * It creates a page that fully fits the page.
 * It also has it's own master page which will also be deleted when
 * the page is deleted.
 */
class SCEndOfSlideShowPage : public SCPage
{
public:
    SCEndOfSlideShowPage(const QRectF &screenRect, SCDocument * document);
    ~SCEndOfSlideShowPage();
};

#endif /* KPRENDOFSLIDESHOWPAGE_H */
