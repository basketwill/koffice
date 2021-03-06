/* This file is part of the KDE project
 * Copyright (C) 2007-2011 Thomas Zander <zander@kde.org>
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
#ifndef KTEXTANCHOR_H
#define KTEXTANCHOR_H

#include "KInlineObject.h"

#include "kodftext_export.h"

#include <QPointF>

class KShape;
class KTextAnchorPrivate;
class KXmlElement;
class KShapeLoadingContext;

/**
 * This class is the object that is positioned in the text to be an anchor for a shape.
 * An anchor is the connection between the text-shape and the so called 'anchored-shape', where the
 * anchored shape can be any kind of shape.  This textanchor then connects the anchored-shape
 * to the text flow so the anchored shape can be repositioned on the canvas if new text is inserted
 * or removed before the anchor character.
 *
 * The KTextAnchor object is inserted in text, and is represented to the user as one invisible character
 * in the text flow. Since this is a real character it will be positioned by the text-layout engine and
 * anything that will change the position of the text will thus also change the KTextAnchor character.
 * In such a case where the KTextAnchor character is repositioned the position of the anchored-frame
 * will also be reconsidered.
 *
 * The anchored shape will be positioned (in supporting applications) based on the properties set on the
 * anchor.  The setAlignment(AnchorVertical) and setAlignment(AnchorHorizontal) calls will determine the
 * resulting position relative to the position the KTextAnchor character.
 *
 * Steps to use a KTextAnchor are; <ol>
 * <li> Create a new instance with e.g. new KTextAnchor(myshape);
 * <li> Use loadOdf() to load additional attributes like the "text:anchor-type"
 * <li> Position the anchor with updatePosition() what will attach the KTextAnchor-instance to
 *    the TextShape's \a KTextShapeContainerModel . </ol>
 * The position of the shape relative to the anchor is called the offset. It's loaded by loadOdf().
 * @see KWAnchorStrategy for more information about the layout of anchors/shapes in KWord.
 */
class KODFTEXT_EXPORT KTextAnchor : public KInlineObject
{
public:
    /// the vertical alignment options for the shape this anchor holds.
    enum AnchorVertical {
        TopOfPage,          ///< Align the anchors top to the top of the page
        BottomOfPage,       ///< Align the anchors bottom to the bottom of the page
        TopOfPageContent,   ///< Align the anchors top to the top of the page content (top margin of page)
        BottomOfPageContent,///< Align the anchors bottom to the bottom of the content (bottom margin of page)
        TopOfFrame,         ///< Align the anchors top to the top of the frame it is laid-out in.
        BottomOfFrame,      ///< Align the anchors bottom to the bottom of the frame.
        TopOfParagraph,     ///< Align the anchors top to the top of the paragraph it is anchored in.
        BottomOfParagraph,  ///< Align the anchors bottom to the bottom of the paragraph it is anchord in.
        AboveCurrentLine,   ///< Align the anchors top to the top of the line it is anchord in.
        BelowCurrentLine,   ///< Align the anchors bottom to the bottom of the line it is anchord in.
        VerticalOffset      ///< Move the anchor to be an exact vertical distance from the (baseline) of the anchor.
    };

    /// the horizontal alignment options for the shape this anchor holds.
    enum AnchorHorizontal {
        LeftOfPage,         ///< Align the anchors left to the left of the page
        RightOfPage,        ///< Align the anchors right to the right of the page
        CenterOfPage,       ///< Align the anchors center to the center of the page
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
    KTextAnchor(KShape *shape);
    virtual ~KTextAnchor();

    /**
     * Return the shape that is linked to from the text anchor.
     */
    KShape *shape() const;

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

    /// reimplemented from KInlineObject
    virtual void updatePosition(QTextInlineObject object, const QTextCharFormat &format);
    /// reimplemented from KInlineObject
    virtual void resize(QTextInlineObject object, const QTextCharFormat &format, QPaintDevice *pd);
    /// reimplemented from KInlineObject
    virtual void paint(QPainter &painter, QPaintDevice *pd, const QRectF &rect,
            QTextInlineObject object, const QTextCharFormat &format);

    /// return the offset of the shape from the anchor.
    const QPointF &offset() const;
    /// set the new offset of the shape. Causes a new layout soon.
    void setOffset(const QPointF &offset);

    /// Load the additional attributes.
    bool loadOdf(const KXmlElement &element, KShapeLoadingContext &context);
    /// Save the additional attributes.
    void saveOdf(KShapeSavingContext &context);

    /**
     * Returns true if the anchored frame is positioned as a big character in the text layout
     * or false when it will not take any space as an inline object.
     * An anchor with HorizontalOffset/VerticalOffset as alignment and an offset() that
     * keeps it inside the text size it is placed on will act as a big character in the
     * text flow potentially changing the ascent/descent of the line.
     */
    bool isPositionedInline() const;

    /// \internal make sure that the anchor has no KTextShapeContainerModel references anymore.
    void detachFromModel();

private:
    Q_DECLARE_PRIVATE(KTextAnchor)
};

#endif
