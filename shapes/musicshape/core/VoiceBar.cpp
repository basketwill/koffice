/* This file is part of the KDE project
 * Copyright (C) 2007 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>
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
#include "VoiceBar.h"
#include "MusicElement.h"
#include <QtCore/QList>

namespace MusicCore {

class VoiceBar::Private
{
public:
    QList<MusicElement*> elements;
};

VoiceBar::VoiceBar() : d(new Private)
{
}

VoiceBar::~VoiceBar()
{
    Q_FOREACH(MusicElement* me, d->elements) delete me;
    delete d;
}

int VoiceBar::elementCount() const
{
    return d->elements.size();
}

MusicElement* VoiceBar::element(int index)
{
    Q_ASSERT( index >= 0 && index < elementCount() );
    return d->elements[index];
}

void VoiceBar::addElement(MusicElement* element)
{
    Q_ASSERT( element );
    d->elements.append(element);
}

void VoiceBar::insertElement(MusicElement* element, int before)
{
    Q_ASSERT( element );
    Q_ASSERT( before >= 0 && before <= elementCount() );
    d->elements.insert(before, element);
}

void VoiceBar::insertElement(MusicElement* element, MusicElement* before)
{
    Q_ASSERT( element );
    Q_ASSERT( before );
    int index = d->elements.indexOf(before);
    Q_ASSERT( index != -1 );
    insertElement(element, index);
}

void VoiceBar::removeElement(int index, bool deleteElement)
{
    Q_ASSERT( index >= 0 && index < elementCount() );
    MusicElement* e = d->elements.takeAt(index);
    if (deleteElement) {
        delete e;
    }
}

void VoiceBar::removeElement(MusicElement* element, bool deleteElement)
{
    Q_ASSERT( element );
    int index = d->elements.indexOf(element);
    Q_ASSERT( index != -1 );
    removeElement(index, deleteElement);
}

} // namespace MusicCore
