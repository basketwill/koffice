/* This file is part of the KDE project
 * Copyright (C) 2006-2011 Thomas Zander <zander@kde.org>
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
#ifndef KSTYLEMANAGERPRIVATE_P_H
#define KSTYLEMANAGERPRIVATE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the KOdfText API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QHash>
#include <QList>
#include <QMap>

class KODFTEXT_TEST_EXPORT KStyleManagerPrivate
{
public:
    KStyleManagerPrivate();
    ~KStyleManagerPrivate();
    void refreshUnsetStoreFor(int key);
    void requestFireUpdate(KStyleManager *q);
    void updateAlteredStyles(); // slot for the QTimer::singleshot

    /// recursively add all char styles under \a ps
    void requestUpdateForChildren(KParagraphStyle *ps);


    static int s_stylesNumber; // For giving out unique numbers to the styles for referencing

    QHash<int, KCharacterStyle*> charStyles;
    QHash<int, KParagraphStyle*> paragStyles;
    QHash<int, KListStyle*> listStyles;
    QHash<int, KListStyle *> automaticListStyles;
    QHash<int, KTableStyle *> tableStyles;
    QHash<int, KTableColumnStyle *> tableColumnStyles;
    QHash<int, KTableRowStyle *> tableRowStyles;
    QHash<int, KTableCellStyle *> tableCellStyles;
    QHash<int, KSectionStyle *> sectionStyles;
    QList<ChangeFollower*> documentUpdaterProxies;

    /// the unsetStore has a hash from the styleId to a set of all the property ids of a style
    QHash<int, QMap<int, QVariant> > unsetStore;

    bool updateTriggered;
    /// the updateQueue contains a map of (altered) style-ids to a state of the style
    QMap<int, QMap<int, QVariant> > updateQueue;

    KParagraphStyle *defaultParagraphStyle;
    KListStyle *defaultListStyle;
    KListStyle *outlineStyle;
};

#endif
