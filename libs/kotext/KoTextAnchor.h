/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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
#ifndef KOTEXTANCHOR_H
#define KOTEXTANCHOR_H

#include "KoInlineObject.h"

#include <kotext_export.h>

#include <QPointF>

class KoShape;

/**
 * This class is the object that is positioned in the text to be an anchor for a shape.
 * The idea is that when the text is relayouted and this inline character is moved, the shape
 * associated with it is repositioned based on a set of rules influenced by the data on this anchor.
 */
class KOTEXT_EXPORT KoTextAnchor : public KoInlineObject {
public:
    /// the vertical alignment options for the shape this anchor holds.
    enum AnchorVertical {
        TopOfFrame,         ///< Align the anchors top to the top of the frame it is laid-out in.
        TopOfParagraph,     ///< Align the anchors top to the top of the paragraph it is anchored in.
        AboveCurrentLine,   ///< Align the anchors top to the top of the line it is anchord in.
        BelowCurrentLine,   ///< Align the anchors bottom to the bottom of the line it is anchord in.
        BottomOfParagraph,  ///< Align the anchors bottom to the bottom of the paragraph it is anchord in.
        BottomOfFrame,      ///< Align the anchors bottom to the bottom of the frame.
        VerticalOffset      ///< Move the anchor to be an exact vertical distance from the (baseline) of the anchor.
    };
    /// the horizontal alignment options for the shape this anchor holds.
    enum AnchorHorizontal {
        Left,               ///< Align the anchors left to the left of the frame it is laid-out in.
        Right,              ///< Align the anchors rigth to the rigth of the frame it is laid-out in.
        Center,             ///< Align the anchors center to the center of the frame it is laid-out in.
        ClosestToBinding,   ///< Like Left when on an odd page, or Right otherwise.
        FurtherFromBinding, ///< Like Left when on an even page, or Right otherwise.
        HorizontalOffset    ///< Move the anchor to be an exact horizontal distance from the the anchor.
    };

    /**
     * Constructor for an in-place anchor.
     * @param shape the anchored shape that this anchor links to.
     */
    KoTextAnchor(KoShape *shape);
    ~KoTextAnchor();

    /**
     * Return the shape that is linked to from the text anchor.
     */
    KoShape *shape() const;

    /**
     * The linked shape will be placed based on the combined horizontal and vertical alignments.
     * Setting the alignment will trigger a relayout of the text and soon after reposition the
     * anchored shape.
     * @param horizontal the new horizontal alignment
     */
    void setAlignment(AnchorHorizontal horizontal);
    /**
     * The linked shape will be placed based on the combined horizontal and vertical alignments.
     * Setting the alignment will trigger a relayout of the text and soon after reposition the
     * anchored shape.
     * @param vertical the new vertical alignment
     */
    void setAlignment(AnchorVertical vertical);

    /// return the current vertical aligment
    AnchorVertical verticalAlignment() const;

    /// return the current horizontal aligment
    AnchorHorizontal horizontalAlignment() const;

    /// returns the cursor position in the document where this anchor is positioned.
    int positionInDocument() const;

    /// returns the document that this anchor is associated with.
    const QTextDocument *document() const;

    /// reimplemented from KoInlineObject
    virtual void updatePosition(const QTextDocument *document, QTextInlineObject object,
            int posInDocument, const QTextCharFormat &format);
    /// reimplemented from KoInlineObject
    virtual void resize(const QTextDocument *document, QTextInlineObject object,
            int posInDocument, const QTextCharFormat &format, QPaintDevice *pd);
    /// reimplemented from KoInlineObject
    virtual void paint (QPainter &painter, QPaintDevice *pd, const QTextDocument *document,
            const QRectF &rect, QTextInlineObject object, int posInDocument, const QTextCharFormat &format);

    /// return the offset from the origin.
    const QPointF &offset() const;
    /// set the new offset. Causes a new layout soon.
    void setOffset(const QPointF &offset);

    void saveOdf (KoShapeSavingContext & context);
private:
    class Private;
    Private * const d;
};

#endif
