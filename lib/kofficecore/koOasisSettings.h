/* This file is part of the KDE project
   Copyright (C) 2004 Laurent Montel \<montel@kde.org\>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/


#ifndef KOOASISSETTINGS_H
#define KOOASISSETTINGS_H

#include <qdom.h>

/**
 * @brief Parse settings.xml file.
 *
 * This class helps parsing the settings.xml file of an OASIS document.
 *
 * For reference, the structure of settings.xml looks like:
 * <pre>
 *   \<office:settings\>
 *      \<config:config-item-set config:name="view-settings"\>
 *         \<config:config-item-map-indexed config:name="Views"\>
 *           \<config:config-item-map-entry\>
 *             \<config:config-item config:name="SnapLinesDrawing" config:type="string"\>value\</config:config-item\>
 *               ....
 *                \<config:config-item-map-named config:name="Tables"\>
 *                  \<config:config-item-map-entry config:name="Sheet1"\>
 *                    \<config:config-item config:name="CursorPositionX"\>
 *                    ......
 *                  \</config:config-item-map-entry\>
 *                  \<config:config-item-map-entry config:name="Sheet2"\>
 *                  ....
 *                  \</config:config-item-map-entry\>
 *                \</config:config-item-map-named\>
 *           .....
 *           \</config:config-item-map-entry\>
 *         \</config:config-item-map-indexed\>
 *         \<config:config-item-map-indexed config:name="Interface"\>
 *         .......
 *         \</config:config-item-map-indexed\>
 *       \</config:config-item-set\>
 *       \<config:config-item-set config:name="configure-settings"\>
 *       ....
 *       \</config:config-item-set\>
 *   \</office:settings\>
 * </pre>
 * Basically, config-items are either part of an item-set, or part of an item-map which is inside an item-set.
 *
 * The API of KoOasisSettings allows the caller to look for a given item-set or item-map once,
 * and then lookup multiple items inside it.
 */
class KoOasisSettings
{
public:
    KoOasisSettings( const QDomDocument &doc );

    /**
     * Select the config-item-set named @p configItemName
     * @return false if no such item set was found
     */
    bool selectItemSet( const QString &itemSetName );

    /**
     * Select the config-item-map-indexed named @p mapItemName, (for instance
     * inside the item-set previously selected by selectItemSet).
     *
     * An indexed map is an array (or sequence), i.e. items are supposed to
     * be retrieved by index. TODO: some API for that.
     * @return false if no such map was found
     */
    bool selectItemMap( const QString &itemMapName );

    /**
     * Select the config-item-map-named named @p mapItemName, inside the item-set
     * previously selected by selectItemSet.
     *
     * A named map is a map where items are retrieved by entry name, @see selectItemMapEntry
     * @return false if no such map was found
     */
    bool selectItemMapNamed( const QString &itemMapName );

    /**
     * Select an entry in a named map
     */
    bool selectItemMapEntry( const QString& entryName );

    // TODO: remove last argument, use selectItemMapEntry instead
    int parseConfigItemInt( const QString & configName, const QString &itemNameEntry = QString::null ) const;
    double parseConfigItemDouble( const QString & configName, const QString &itemNameEntry = QString::null ) const;
    QString parseConfigItemString( const QString & configName, const QString &itemNameEntry = QString::null ) const;
    bool parseConfigItemBool( const QString & configName, const QString &itemNameEntry = QString::null ) const;
    short parseConfigItemShort( const QString & configName, const QString &itemNameEntry = QString::null ) const;
    long parseConfigItemLong( const QString & configName, const QString &itemNameEntry = QString::null ) const;

private:
    /// @internal
    QString parseConfigItem( const QString &item, const QString &itemNameEntry = QString::null ) const;
    /// @internal
    QString parseConfigItemName( const QDomElement & element, const QString &item ) const;

    const QDomDocument m_doc;
    QDomElement m_element;
};

#endif
