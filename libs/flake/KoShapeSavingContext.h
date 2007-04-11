/* This file is part of the KDE project
   Copyright (C) 2004-2006 David Faure <faure@kde.org>
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
 
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or ( at your option ) any later version.
 
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
 
   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOSHAPESAVINGCONTEXT_H
#define KOSHAPESAVINGCONTEXT_H

#include <flake_export.h>

#include <QFlags>
#include <QMap>
#include <QString>

class KoShape;
class KoXmlWriter;
class KoSavingContext;
class KoGenStyles;

/**
 * The set of data for the ODF file format used during saving of a shape.
 */
class FLAKE_EXPORT KoShapeSavingContext
{
public:
    /// The Style used for saving the shape
    enum ShapeSavingOption
    {
        /**
         * If set the style of family presentation is used, when not set the
         * family graphic is used.
         * See OpenDocument 9.2.15 Common Drawing Shape Attributes / Style
         */
        PresentationShape = 1,
        /**
         * Save the draw:id used for referencing the shape.
         * See OpenDocument 9.2.15 Common Drawing Shape Attributes / ID
         */
        DrawId = 2,
        /**
         * If set the automatic style will be marked as being needed in styles.xml
         */
        AutoStyleInStyleXml = 4,
        FrameOpened = 8     ///< If a 'draw:frame' tag has been openened shape properties should be saved differently
    };
    Q_DECLARE_FLAGS( KoShapeSavingOptions, ShapeSavingOption )

    /**
     * @brief Constructor
     * @param xmlWriter used for writing the xml
     * @param context the saveing context used
     */
    KoShapeSavingContext( KoXmlWriter &xmlWriter, KoSavingContext &context );
    ~KoShapeSavingContext();

    /**
     * @brief Get the xml writer
     *
     * @return xmlWriter
     */
    KoXmlWriter & xmlWriter();

    /**
     * @brief Set the xml writer
     *
     * Change the xmlWriter that is used in the Context e.g. for saving to styles.xml 
     * instead of content.xml
     *
     * @param xmlWriter to use
     */
    void setXmlWriter( KoXmlWriter & xmlWriter );

    /**
     * @brief Get the main styles
     *
     * @return main styles 
     */
    KoGenStyles & mainStyles();

    /**
     * @brief Check if an option is set
     *
     * @return ture if the option is set, false otherwise
     */
    bool isSet( ShapeSavingOption option ) const;

    /**
     * @brief Set the options to use
     *
     * @param options to use
     */
    void setOptions( KoShapeSavingOptions options );

    void addOption( ShapeSavingOption option);

    void removeOption( ShapeSavingOption option);

    /**
     * @brief Get the options used
     *
     * @return options used
     */
    KoShapeSavingOptions options() const;

    /**
     * @brief Get the draw id for a shape
     *
     * The draw:id is unique for all shapes.
     *
     * @param shape for which the draw id should be returned
     * @param insert if true a new draw id will be generated if there is non yet
     *
     * @return the draw id for the shape or and empty string if it was not found
     */
    const QString drawId( const KoShape * shape, bool insert = true );

protected:
    KoXmlWriter *m_xmlWriter;
    KoSavingContext &m_context;

    KoShapeSavingOptions m_savingOptions;

    QMap<const KoShape *, QString> m_drawIds;
    int m_drawId;
    // TODO handle relative positions
};

Q_DECLARE_OPERATORS_FOR_FLAGS( KoShapeSavingContext::KoShapeSavingOptions )

#endif // KOSHAPESAVINGCONTEXT_H
