/* This file is part of the KDE project
   Copyright (C) 2005 David Faure <faure@kde.org>
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>

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
#ifndef KODFSTOREREADER_H
#define KODFSTOREREADER_H

#include "flake_export.h"

class QString;
class QIODevice;
class QXmlStreamReader;
class KOdfStore;
class KOdfStylesReader;
class KXmlDocument;
class KXmlElement;

/**
 * Helper class around KOdfStore for reading out ODF files.
 *
 * The class loades and parses files from the KOdfStore.
 *
 * @author: David Faure <faure@kde.org>
 */
class FLAKE_EXPORT KOdfStoreReader
{
public:
    /// @param store recontents the property of the caller
    explicit KOdfStoreReader(KOdfStore* store);

    ~KOdfStoreReader();

    /**
     * Get the store
     */
    KOdfStore* store() const;

    /**
     * Get the styles
     *
     * To get a usable result loadAndParse( QString ) has to be called first.
     *
     * @return styles
     */
    KOdfStylesReader &styles();

    /**
     * Get the content document
     *
     * To get a usable result loadAndParse( QString ) has to be called first.
     *
     * This gives you the content of the content.xml file
     */
    KXmlDocument contentDoc() const;

    /**
     * Get the settings document
     *
     * To get a usable result loadAndParse( QString ) has to be called first.
     *
     * This gives you the content of the settings.xml file
     */
    KXmlDocument settingsDoc() const;

    /**
     * Load and parse
     *
     * This function loads and parses the content.xml, styles.xml and the settings.xml
     * file in the store. The sytles are already parsed.
     *
     * After this function is called you can access the data via
     * styles()
     * contentDoc()
     * settingsDoc()
     *
     * @param errorMessage The errorMessage is set in case an error is encounted.
     * @return true if loading and parsing was successful, false otherwise. In case of an error
     * the errorMessage is updated accordingly.
     */
    bool loadAndParse(QString &errorMessage);

    /**
     * Load a file from an odf store
     */
    bool loadAndParse(const QString &fileName, KXmlDocument &doc, QString &errorMessage);

    /**
     * Load a file and parse from a QIODevice
     * filename argument is just used for debug message
     */
    static bool loadAndParse(QIODevice *fileDevice, KXmlDocument &doc, QString &errorMessage, const QString& fileName);

    /**
     * Get mimetype from full path, using the manifest
     */
    static QString mimeForPath(const KXmlDocument &doc, const QString &fullPath);

private:
    class Private;
    Private * const d;
};

#endif /* KODFSTOREREADER_H */
