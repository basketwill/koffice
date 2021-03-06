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

#ifndef SCSOUNDEVENTACTION_H
#define SCSOUNDEVENTACTION_H

#include <KEventAction.h>

#include <QObject>

namespace Phonon {
    class MediaObject;
}

class SCSoundData;

#define SCSoundEventActionId "SCSoundEventAction"

class SCSoundEventAction : public QObject, public KEventAction
{
    Q_OBJECT
public:
    SCSoundEventAction();
    virtual ~SCSoundEventAction();

    virtual bool loadOdf(const KXmlElement & element, KShapeLoadingContext &context);
    virtual void saveOdf(KShapeSavingContext & context) const;

    virtual void start();
    virtual void finish();

    void setSoundData(SCSoundData * soundData);
    SCSoundData * soundData() const;

public slots:
    void finished();

private:
    Phonon::MediaObject * m_media;
    SCSoundData *m_soundData;
};

#endif /* SCSOUNDEVENTACTION_H */
