/* This file is part of the KDE project
 * Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
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

#ifndef KDELETECHANGEMARKER_H
#define KDELETECHANGEMARKER_H

#include "kodftext_export.h"

#include <KInlineObject.h>
#include <QTextFormat>
#include <KListStyle.h>

class KChangeTracker;
class KXmlElement;

class QTextDocument;

class KODFTEXT_EXPORT KDeleteChangeMarker : public KInlineObject
{
public:

    typedef enum {
        DeletedListItem = QTextFormat::UserProperty + 9999,
        DeletedList,
    } listDeleteStatus;

    /************************************ODF Bug (List Delete Changes) Workaround code *****************************/
    static const QString RDFListName;
    static const QString RDFListItemName;
    static const QString RDFListValidity;
    static const QString RDFListItemValidity;
    static const QString RDFListLevel;
    static const QString RDFDeleteChangeContext;
    /****************************************************************************************************************/

    KDeleteChangeMarker(KChangeTracker *changeTracker);

    virtual ~KDeleteChangeMarker();

    /**
    * Store the deleted text.
    * @param text the deleted text
    */
//    void setText(const QString &text);

    /// return the deleted text
//    QString text() const;

    /**
    * Set the changeTracker id for this change.
    * @param id the new id
    */
    void setChangeId(int id);

    /// return the change id
    int changeId() const;

    ///Return the position of the marker in the document
    int position() const;

    virtual bool loadOdf(const KXmlElement &element, KShapeLoadingContext &context);
    ///reimplemented
    virtual void saveOdf(KShapeSavingContext &context);

    void setDeleteChangeXml(QString &deleteChangeXml);

    KListStyle *deletedListStyle(KListStyle::ListIdType id);

    void setDeletedListStyle(KListStyle::ListIdType, KListStyle *style);

protected:

    virtual void paint(QPainter &painter, QPaintDevice *pd, const QRectF &rect, QTextInlineObject object, const QTextCharFormat &format);

    virtual void resize(QTextInlineObject object, const QTextCharFormat &format, QPaintDevice *pd);

    virtual void updatePosition(QTextInlineObject object, const QTextCharFormat &format);

private:

    class Private;
    Private * const d;
};

#endif // KDELETECHANGEMARKER_H
