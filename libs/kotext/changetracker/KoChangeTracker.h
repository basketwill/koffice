/* This file is part of the KDE project
 * Copyright (C) 2008 Pierre Stirnweiss \pierre.stirnweiss_koffice@gadz.org>
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
#ifndef KOCHANGETRACKER_H
#define KOCHANGETRACKER_H

#include <QObject>
#include <QMetaType>
#include <QTextCharFormat>
#include <QTextFormat>
#include <QHash>
#include <QString>
#include <QList>

#include "kotext_export.h"
#include <KoDataCenter.h>
#include "KoChangeTrackerElement.h"

#include <KoGenChange.h>
#include <KoGenChanges.h>

class KOTEXT_EXPORT KoChangeTracker : public QObject//, public KoDataCenter
{
    Q_OBJECT
public:
    KoChangeTracker(QObject *parent = 0);
    
    ~KoChangeTracker();

    int getFormatChangeId(QString title, QTextFormat &format, QTextFormat &prevFormat, int existingChangeId);
    int getInsertChangeId(QString title, int existingChangeId);
    
    bool containsInlineChanges(const QTextFormat &format);
    bool saveInlineChange(int changeId, KoGenChange &change);

private:
   
    class Private;
    Private* const d;
};

Q_DECLARE_METATYPE(KoChangeTracker*)

#endif
