/* This file is part of the KDE project
   Copyright (C) 2004 Laurent Montel <montel@kde.org>
                      David Faure <faure@kde.org>

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
#include <koffice_export.h>

/**
 * @brief Parse settings.xml file.
 *
 * This class helps parsing the settings.xml file of an OASIS document.
 *
 * For reference, the structure of settings.xml looks like:
 * <pre>
 *   \<office:settings\>
 *      \<config:config-item-set config:name="configure-settings"\>
 *      ....
 *      \</config:config-item-set\>
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
 *      \</config:config-item-set\>
 *   \</office:settings\>
 * </pre>
 * Basically, an item-set is a set of named \<config-item\>s and/or maps.
 * There are two kinds of maps (by-index or by-name), and entries in the
 * maps contain \<config-item\>s too, or nested maps.
 *
 * The API of KoOasisSettings allows the caller to look for a given item-set
 * or item-map once, and then lookup multiple items inside it.
 * It also allows "drilling down" inside the tree in case of nesting.
 */
class KOFFICECORE_EXPORT KoOasisSettings
{
public:
    KoOasisSettings( const QDomDocument& doc );

    class Items;

    /**
     * Returns the toplevel item-set named @p itemSetName.
     * If not found, the returned items instance is null.
     */
    Items itemSet( const QString& itemSetName ) const;

    class IndexedMap;
    class NamedMap;
    /// Represents a collection of items (config-item or maps).
    class KOFFICECORE_EXPORT Items
    {
        friend class KoOasisSettings;
        friend class IndexedMap;
        friend class NamedMap;
        Items( const QDomElement& elem ) : m_element( elem ) {}
    public:
        bool isNull() const { return m_element.isNull(); }

        /**
         * Look for the config-item-map-indexed named @p itemMapName and return it.
         *
         * An indexed map is an array (or sequence), i.e. items are supposed to
         * be retrieved by index. This is useful for e.g. "view 0", "view 1" etc.
         */
        IndexedMap indexedMap( const QString& itemMapName ) const;

        /**
         * Look for the config-item-map-named named @p mapItemName and return it.
         *
         * A named map is a map where items are retrieved by entry name, @see selectItemMapEntry
         * @return false if no such map was found
         */
        NamedMap namedMap( const QString& itemMapName ) const;

        int parseConfigItemInt( const QString& configName, int defValue = 0 ) const;
        double parseConfigItemDouble( const QString& configName, double defValue = 0 ) const;
        QString parseConfigItemString( const QString& configName, const QString& defValue = QString::null ) const;
        bool parseConfigItemBool( const QString& configName, bool defValue = false ) const;
        short parseConfigItemShort( const QString& configName, short defValue = 0 ) const;
        long parseConfigItemLong( const QString& configName, long defValue = 0 ) const;
    private:
        /// @internal
        QString findConfigItem( const QString& item, bool* ok ) const;
        /// @internal
        static QString findConfigItem( const QDomElement& element, const QString& item, bool* ok );

        QDomElement m_element;
    };

    /// Internal base class for IndexedMap and NamedMap
    class Map
    {
    public:
        bool isNull() const { return m_element.isNull(); }
    protected:
        Map( const QDomElement& elem ) : m_element( elem ) {}
        const QDomElement m_element;
    };

    class KOFFICECORE_EXPORT IndexedMap : public Map
    {
        friend class Items;
        IndexedMap( const QDomElement& elem ) : Map( elem ) {}
    public:
        /// Returns an entry in an indexed map
        Items entry( int entryIndex ) const;
    };

    class KOFFICECORE_EXPORT NamedMap : public Map
    {
        friend class Items;
        NamedMap( const QDomElement& elem ) : Map( elem ) {}
    public:
        /// Returns an entry in a named map
        Items entry( const QString& entryName ) const;
    };

private:
    const QDomElement m_settingsElement;
};

#endif
