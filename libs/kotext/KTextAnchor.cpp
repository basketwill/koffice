/* This file is part of the KDE project
 * Copyright (C) 2007-2011 Thomas Zander <zander@kde.org>
 * Copyright (C) 2010 Ko Gmbh <casper.boemann@kogmbh.com>
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

#include "KTextAnchor.h"
#include "KInlineObject_p.h"
#include "KTextDocumentLayout.h"
#include "KTextShapeContainerModel.h"
#include "KTextShapeData.h"
#include "KOdfStyleStack.h"
#include "KOdfLoadingContext.h"

#include <KShapeContainer.h>
#include <KXmlWriter.h>
#include <KXmlReader.h>
#include <KOdfXmlNS.h>
#include <KShapeSavingContext.h>
#include <KShapeLoadingContext.h>
#include <KUnit.h>

#include <QTextInlineObject>
#include <QFontMetricsF>
#include <QPainter>
#include <KDebug>

#include "changetracker/KChangeTracker.h"
#include "changetracker/KChangeTrackerElement.h"
#include "styles/KCharacterStyle.h"
#include "KTextDocument.h"
#include <KOdfGenericChanges.h>

// #define DEBUG_PAINTING

class KTextAnchorPrivate : public KInlineObjectPrivate
{
public:
    KTextAnchorPrivate(KTextAnchor *qq, KShape *s)
            : KInlineObjectPrivate(qq),
            shape(s),
            horizontalAlignment(KTextAnchor::HorizontalOffset),
            verticalAlignment(KTextAnchor::VerticalOffset),
            model(0),
            isPositionedInline(false)
    {
        Q_ASSERT(shape);
    }

    void relayout()
    {
        if (document && shape->parent()) {
            KTextShapeData *data  = qobject_cast<KTextShapeData*>(shape->parent()->userData());
            Q_ASSERT(data);
            data->foul();
            KTextDocumentLayout *lay = qobject_cast<KTextDocumentLayout*>(document->documentLayout());
            if (lay)
                lay->interruptLayout();
            data->fireResizeEvent();
        }
    }

    /// as multiple shapes can hold 1 text flow; the anchored shape can be moved between containers and thus models
    void setContainer(KShapeContainer *container)
    {
        Q_Q(KTextAnchor);
        if (container == 0) {
            if (model)
                model->removeAnchor(q);
            model = 0;
            shape->setParent(0);
            return;
        }
        KTextShapeContainerModel *theModel = dynamic_cast<KTextShapeContainerModel*>(container->model());
        if (theModel != model) {
            if (model)
                model->removeAnchor(q);
            if (shape->parent() != container) {
                if (shape->parent()) {
                    shape->parent()->removeShape(shape);
                }
                container->addShape(shape);
            }
            model = theModel;
            model->addAnchor(q);
        }
        Q_ASSERT(model == theModel);
    }

    QDebug printDebug(QDebug dbg) const
    {
#ifndef NDEBUG
        dbg.nospace() << "KTextAnchor";
        dbg.space() << anchorPosition();
        dbg.space() << "offset:" << distance;
        dbg.space() << "shape:" << shape->name();
#endif
        return dbg.space();
    }

    QString anchorPosition() const
    {
        QString answer;
        switch (verticalAlignment) {
        case KTextAnchor::TopOfFrame: answer = "TopOfFrame"; break;
        case KTextAnchor::TopOfParagraph: answer = "TopOfParagraph"; break;
        case KTextAnchor::AboveCurrentLine: answer = "AboveCurrentLine"; break;
        case KTextAnchor::BelowCurrentLine: answer = "BelowCurrentLine"; break;
        case KTextAnchor::BottomOfParagraph: answer = "BottomOfParagraph"; break;
        case KTextAnchor::BottomOfFrame: answer = "BottomOfFrame"; break;
        case KTextAnchor::VerticalOffset: answer = "VerticalOffset"; break;
        case KTextAnchor::TopOfPage: answer = "TopOfPage"; break;
        case KTextAnchor::BottomOfPage: answer = "BottomOfPage"; break;
        case KTextAnchor::TopOfPageContent: answer = "TopOfPageContent"; break;
        case KTextAnchor::BottomOfPageContent: answer = "BottomOfPageContent"; break;
        }
        answer += '|';
        switch(horizontalAlignment) {
        case KTextAnchor::Left: answer+= "Left"; break;
        case KTextAnchor::Right: answer+= "Right"; break;
        case KTextAnchor::Center: answer+= "Center"; break;
        case KTextAnchor::ClosestToBinding: answer+= "ClosestToBinding"; break;
        case KTextAnchor::FurtherFromBinding: answer+= "FurtherFromBinding"; break;
        case KTextAnchor::HorizontalOffset: answer+= "HorizontalOffset"; break;
        case KTextAnchor::LeftOfPage: answer+= "LeftOfPage"; break;
        case KTextAnchor::RightOfPage: answer+= "RightOfPage"; break;
        case KTextAnchor::CenterOfPage: answer+= "CenterOfPage"; break;
        }
        return answer;
    }

    KShape * const shape;
    KTextAnchor::AnchorHorizontal horizontalAlignment;
    KTextAnchor::AnchorVertical verticalAlignment;
    QTextCharFormat format;
    KTextShapeContainerModel *model;
    QPointF distance;
    bool isPositionedInline;

    Q_DECLARE_PUBLIC(KTextAnchor)
};

KTextAnchor::KTextAnchor(KShape *shape)
    : KInlineObject(*(new KTextAnchorPrivate(this, shape)), false)
{
}

KTextAnchor::~KTextAnchor()
{
    Q_D(KTextAnchor);
    if (d->model)
        d->model->removeAnchor(this);
}

KShape *KTextAnchor::shape() const
{
    Q_D(const KTextAnchor);
    return d->shape;
}

void KTextAnchor::setAlignment(KTextAnchor::AnchorHorizontal horizontal)
{
    Q_D(KTextAnchor);
    if (d->horizontalAlignment == horizontal)
        return;
    d->horizontalAlignment = horizontal;
    d->relayout();
}

void KTextAnchor::setAlignment(KTextAnchor::AnchorVertical vertical)
{
    Q_D(KTextAnchor);
    if (d->verticalAlignment == vertical)
        return;
    d->verticalAlignment = vertical;
    d->relayout();
}

KTextAnchor::AnchorVertical KTextAnchor::verticalAlignment() const
{
    Q_D(const KTextAnchor);
    return d->verticalAlignment;
}

KTextAnchor::AnchorHorizontal KTextAnchor::horizontalAlignment() const
{
    Q_D(const KTextAnchor);
    return d->horizontalAlignment;
}

void KTextAnchor::updatePosition(QTextInlineObject object, const QTextCharFormat &format)
{
    Q_UNUSED(object);
    Q_UNUSED(format);
    Q_D(KTextAnchor);
    d->format = format;
    d->setContainer(dynamic_cast<KShapeContainer*>(shapeForPosition(d->document, d->positionInDocument)));
}

void KTextAnchor::resize(QTextInlineObject object, const QTextCharFormat &format, QPaintDevice *pd)
{
    Q_UNUSED(object);
    Q_UNUSED(format);
    Q_UNUSED(pd);
    Q_D(KTextAnchor);

    // important detail; top of anchored shape is at the baseline.
    QFontMetricsF fm(format.font(), pd);
    if (d->horizontalAlignment == HorizontalOffset && d->verticalAlignment == VerticalOffset
            && d->distance.x() == 0
            && d->distance.y() < -fm.ascent() + d->shape->size().height() // not above line
            && d->distance.y() < fm.descent()) { // not below line
        d->isPositionedInline = true;
        object.setWidth(d->shape->size().width());
        object.setAscent(qMax((qreal) 0, -d->distance.y()));
        object.setDescent(qMax((qreal) 0, d->shape->size().height() + d->distance.y()));
    } else {
        d->isPositionedInline = false;
        object.setWidth(0);
        object.setAscent(0);
        object.setDescent(0);
    }
}

void KTextAnchor::paint(QPainter &painter, QPaintDevice *, const QRectF &rect, QTextInlineObject, const QTextCharFormat &)
{
    Q_UNUSED(painter);
    Q_UNUSED(rect);

    // This section of code is to indicate changes done to KoTextAnchors. Once the new approach is complete this can be removed
    // In this approach we draw a rectangle around the shape with the appropriate change indication color.
    Q_D(KTextAnchor);
    int changeId = d->format.property(KCharacterStyle::ChangeTrackerId).toInt();
    bool drawChangeRect = false;

    QRectF changeRect = rect;
    changeRect.adjust(0,0,1,0);
    QPen changePen;
    changePen.setWidth(2);

    // we never paint ourselves; the shape can do that.
#ifdef DEBUG_PAINTING
    painter.setOpacity(0.5);
    QRectF charSpace = rect;
    if (charSpace.width() < 10)
        charSpace.adjust(-5, 0, 5, 0);
    painter.fillRect(charSpace, QColor(Qt::green));
#endif

    KChangeTracker *changeTracker = KTextDocument(document()).changeTracker();
    if (!changeTracker)
        return;

    KChangeTrackerElement *changeElement = changeTracker->elementById(changeId);
    if (changeElement && changeElement->changeType() == KOdfGenericChange::DeleteChange) {
        changePen.setColor(changeTracker->deletionBgColor());
        drawChangeRect = true;
    } else if (changeElement && changeElement->changeType() == KOdfGenericChange::InsertChange) {
        changePen.setColor(changeTracker->insertionBgColor());
        drawChangeRect = true;
    } else if (changeElement && changeElement->changeType() == KOdfGenericChange::FormatChange) {
        changePen.setColor(changeTracker->formatChangeBgColor());
    }

    painter.setPen(changePen);
    if (drawChangeRect && changeTracker->displayChanges())
        painter.drawRect(changeRect);

    // End of Change Visualization Section. Can be removed once the new approach is finalized
}

const QPointF &KTextAnchor::offset() const
{
    Q_D(const KTextAnchor);
    return d->distance;
}

void KTextAnchor::setOffset(const QPointF &offset)
{
    Q_D(KTextAnchor);
    if (d->distance == offset)
        return;
    d->distance = offset;
    d->relayout();
}

void KTextAnchor::saveOdf(KShapeSavingContext &context)
{
    Q_D(KTextAnchor);
    // the anchor type determines where in the stream the shape is to be saved.
    enum OdfAnchorType {
        AsChar,
        Frame,
        Paragraph,
        Undefined
    };
    // ODF is not nearly as powerful as we need it (yet) so lets do some mapping.
    OdfAnchorType odfAnchorType = Undefined;
    switch (d->verticalAlignment) {
    case KTextAnchor::TopOfFrame:
    case KTextAnchor::BottomOfFrame:
        odfAnchorType = Frame;
        break;
    case KTextAnchor::TopOfParagraph:
    case KTextAnchor::AboveCurrentLine:
    case KTextAnchor::BelowCurrentLine:
    case KTextAnchor::BottomOfParagraph:
    case KTextAnchor::TopOfPage:
    case KTextAnchor::BottomOfPage:
    case KTextAnchor::TopOfPageContent:
    case KTextAnchor::BottomOfPageContent:
        odfAnchorType = Paragraph;
        break;
    case KTextAnchor::VerticalOffset:
        odfAnchorType = AsChar;
        break;
    }
    Q_ASSERT(odfAnchorType != Undefined);

    if (odfAnchorType == AsChar) {
       if (qAbs(d->distance.y()) > 1E-4)
           shape()->setAdditionalAttribute("svg:y", QString("%1pt").arg(d->distance.y()));

        // the draw:transform should not have any offset since we put that in the svg:y already.
        context.addShapeOffset(shape(), shape()->absoluteTransformation(0).inverted());

        shape()->setAdditionalAttribute("text:anchor-type", "as-char");
        shape()->saveOdf(context);
        shape()->removeAdditionalAttribute("svg:y");
    } else {
        // these don't map perfectly to ODF because we have more functionality
        shape()->setAdditionalAttribute("koffice:anchor-type", d->anchorPosition());

        QString type;
        if (odfAnchorType == Frame)
            type = "frame";
        else
            type = "paragraph";
        shape()->setAdditionalAttribute("text:anchor-type", type);
        if (shape()->parent()) {// an anchor may not yet have been layout-ed
            QTransform parentMatrix = shape()->parent()->absoluteTransformation(0).inverted();
            QTransform shapeMatrix = shape()->absoluteTransformation(0);;

            qreal dx = d->distance.x() - shapeMatrix.dx()*parentMatrix.m11()
                                       - shapeMatrix.dy()*parentMatrix.m21();
            qreal dy = d->distance.y() - shapeMatrix.dx()*parentMatrix.m12()
                                       - shapeMatrix.dy()*parentMatrix.m22();
            context.addShapeOffset(shape(), QTransform(parentMatrix.m11(),parentMatrix.m12(),
                                                    parentMatrix.m21(),parentMatrix.m22(),
                                                    dx,dy));
        }

        shape()->saveOdf(context);
        context.removeShapeOffset(shape());
    }
}

bool KTextAnchor::loadOdf(const KXmlElement &element, KShapeLoadingContext &context)
{
    Q_D(KTextAnchor);
    d->distance = shape()->position();
    if (! shape()->hasAdditionalAttribute("text:anchor-type"))
        return false;
    QString anchorType = shape()->additionalAttribute("text:anchor-type");

    // load settings from graphic style
     KOdfStyleStack &styleStack = context.odfLoadingContext().styleStack();
     styleStack.save();
     if (element.hasAttributeNS(KOdfXmlNS::draw, "style-name")) {
         context.odfLoadingContext().fillStyleStack(element, KOdfXmlNS::draw, "style-name", "graphic");
         styleStack.setTypeProperties("graphic");
     }
     QString verticalPos = styleStack.property(KOdfXmlNS::style, "vertical-pos");
     QString verticalRel = styleStack.property(KOdfXmlNS::style, "vertical-rel");
     QString horizontalPos = styleStack.property(KOdfXmlNS::style, "horizontal-pos");
     QString horizontalRel = styleStack.property(KOdfXmlNS::style, "horizontal-rel");
     styleStack.restore();

     if (element.hasAttributeNS(KOdfXmlNS::koffice, "anchor-type")) {
         anchorType = element.attributeNS(KOdfXmlNS::koffice, "anchor-type"); // our enriched properties
         QStringList types = anchorType.split('|');
         if (types.count() > 1) {
             QString vertical = types[0];
             QString horizontal = types[1];
             if (vertical == "TopOfFrame")
                 d->verticalAlignment = TopOfFrame;
             else if (vertical == "TopOfParagraph")
                 d->verticalAlignment = TopOfParagraph;
             else if (vertical == "AboveCurrentLine")
                 d->verticalAlignment = AboveCurrentLine;
             else if (vertical == "BelowCurrentLine")
                 d->verticalAlignment = BelowCurrentLine;
             else if (vertical == "BottomOfParagraph")
                 d->verticalAlignment = BottomOfParagraph;
             else if (vertical == "BottomOfFrame")
                 d->verticalAlignment = BottomOfFrame;
             else if (vertical == "VerticalOffset")
                 d->verticalAlignment = VerticalOffset;
             else if (vertical == "TopOfPage")
                 d->verticalAlignment = TopOfPage;
             else if (vertical == "BottomOfPage")
                 d->verticalAlignment = BottomOfPage;
             else if (vertical == "TopOfPageContent")
                 d->verticalAlignment = TopOfPageContent;
             else if (vertical == "BottomOfPageContent")
                 d->verticalAlignment = BottomOfPageContent;

             if (horizontal == "Left")
                 d->horizontalAlignment = Left;
             else if (horizontal == "Right")
                 d->horizontalAlignment = Right;
             else if (horizontal == "Center")
                 d->horizontalAlignment = Center;
             else if (horizontal == "ClosestToBinding")
                 d->horizontalAlignment = ClosestToBinding;
             else if (horizontal == "FurtherFromBinding")
                 d->horizontalAlignment = FurtherFromBinding;
             else if (horizontal == "HorizontalOffset")
                 d->horizontalAlignment = HorizontalOffset;
             else if (horizontal == "LeftOfPage")
                 d->horizontalAlignment = LeftOfPage;
             else if (horizontal == "RightOfPage")
                 d->horizontalAlignment = RightOfPage;
             else if (horizontal == "CenterOfPage")
                 d->horizontalAlignment = CenterOfPage;
             return true;
        }
    }

    if (anchorType == "as-char") {
        // 'as-char' means it's completely inline in the text like any other char
        d->horizontalAlignment = HorizontalOffset;
        d->verticalAlignment = VerticalOffset;
        if (verticalRel == "baseline") {
            if (verticalPos == "top") { //svg:y attribute is ignored
                d->distance.setY(-shape()->size().height());
            } else if (verticalPos == "middle") { //svg:y attribute is ignored
                d->distance.setY(-shape()->size().height()*(qreal)0.5);
            } else if (verticalPos == "bottom") { //svg:y attribute is ignored
                d->distance.setY(0);
            }
        }
    } else if (anchorType == "char") {
        // 'char' means it's relative to the paragraph
        // while 'paragraph' further indicates the anchor is always placed at first char
        d->horizontalAlignment = Left;
        d->verticalAlignment = TopOfParagraph;

        // vertical alignment - conversion from style:vertical-rel,pos to koffice:anchor-type
         if (verticalRel == "char") {
             if (verticalPos == "below") { //svg:y attribute is ignored
                 d->verticalAlignment = BelowCurrentLine;
                 d->distance.setY(-shape()->size().height());
             } else if (verticalPos == "bottom") { //svg:y attribute is ignored
                 d->verticalAlignment = BelowCurrentLine;
                 d->distance.setY(0);
             } else if (verticalPos == "from-top") {
                 d->verticalAlignment = AboveCurrentLine;
             } else if (verticalPos == "middle") { //svg:y attribute is ignored
                 return false; // not posible to do it with koffice:anchor-type
             } else if (verticalPos == "top") { //svg:y attribute is ignored
                 d->verticalAlignment = AboveCurrentLine;
                 d->distance.setY(0);
             }
         } else if (verticalRel == "page") {
             if (verticalPos == "below") { //svg:y attribute is ignored
                 d->verticalAlignment = BottomOfPage;
                 d->distance.setY(-shape()->size().height());
             } else if (verticalPos == "bottom") { //svg:y attribute is ignored
                 d->verticalAlignment = BottomOfPage;
                 d->distance.setY(0);
             } else if (verticalPos == "from-top") {
                 d->verticalAlignment = TopOfPage;
             } else if (verticalPos == "middle") { //svg:y attribute is ignored
                 return false; // not posible to do it with koffice:anchor-type
             } else if (verticalPos == "top") { //svg:y attribute is ignored
                 d->verticalAlignment = TopOfPage;
                 d->distance.setY(0);
             }
         } else if (verticalRel == "page-content") {
             if (verticalPos == "below") { //svg:y attribute is ignored
                 d->verticalAlignment = BottomOfPageContent;
                 d->distance.setY(-shape()->size().height());
             } else if (verticalPos == "bottom") { //svg:y attribute is ignored
                 d->verticalAlignment = BottomOfPageContent;
                 d->distance.setY(0);
             } else if (verticalPos == "from-top") {
                 d->verticalAlignment = TopOfPageContent;
             } else if (verticalPos == "middle") { //svg:y attribute is ignored
                 return false; // not posible to do it with koffice:anchor-type
             } else if (verticalPos == "top") { //svg:y attribute is ignored
                 d->verticalAlignment = TopOfPageContent;
                 d->distance.setY(0);
             }
         } else if (verticalRel == "paragraph") {
             if (verticalPos == "below") { //svg:y attribute is ignored
                 d->verticalAlignment = BottomOfParagraph;
                 d->distance.setY(-shape()->size().height());
             } else if (verticalPos == "bottom") { //svg:y attribute is ignored
                 d->verticalAlignment = BottomOfParagraph;
                 d->distance.setY(0);
             } else if (verticalPos == "from-top") {
                 d->verticalAlignment = TopOfParagraph;
             } else if (verticalPos == "middle") { //svg:y attribute is ignored
                 return false; // not posible to do it with koffice:anchor-type
             } else if (verticalPos == "top") { //svg:y attribute is ignored
                 d->verticalAlignment = TopOfParagraph;
                 d->distance.setY(0);
             }
         } else { //TODO another types if needed
             return false;
         }

         //horizontal alignment - conversion from style:horizontal-rel,pos to koffice:anchor-type
         if (horizontalRel == "char") {
             if (horizontalPos == "center") { //svg:x attribute is ignored
                 return false; // not posible to do it with koffice:anchor-type
             } else if (horizontalPos == "from-inside") {
                 return false; // not posible to do it with koffice:anchor-type
             } else if (horizontalPos == "from-left") {
                 d->horizontalAlignment = HorizontalOffset;
             } else if (horizontalPos == "inside") { //svg:x attribute is ignored
                 return false; // not posible to do it with koffice:anchor-type
             } else if (horizontalPos == "left") { //svg:x attribute is ignored
                 d->horizontalAlignment = HorizontalOffset;
                 d->distance.setX(0);
             } else if (horizontalPos == "outside") { //svg:x attribute is ignored
                 return false; // not posible to do it with koffice:anchor-type
             } else if (horizontalPos == "right") { //svg:x attribute is ignored
                 d->horizontalAlignment = HorizontalOffset;
                 d->distance.setX(-shape()->size().width());
             }
         } else if (horizontalRel == "page") {
             if (horizontalPos == "center") { //svg:x attribute is ignored
                 d->horizontalAlignment = CenterOfPage;
                 d->distance.setX(0);
             } else if (horizontalPos == "from-inside") {
                 return false; // not posible to do it with koffice:anchor-type
             } else if (horizontalPos == "from-left") {
                 d->horizontalAlignment = LeftOfPage;
             } else if (horizontalPos == "inside") { //svg:x attribute is ignored
                 return false; // not posible to do it with koffice:anchor-type
             } else if (horizontalPos == "left") { //svg:x attribute is ignored
                 d->horizontalAlignment = LeftOfPage;
                 d->distance.setX(0);
             } else if (horizontalPos == "outside") { //svg:x attribute is ignored
                 return false; // not posible to do it with koffice:anchor-type
             } else if (horizontalPos == "right") { //svg:x attribute is ignored
                 d->horizontalAlignment = RightOfPage;
                 d->distance.setX(0);
             }
         } else if (horizontalRel == "page-content") {
             if (horizontalPos == "center") { //svg:x attribute is ignored
                 return false; // not posible to do it with koffice:anchor-type
             } else if (horizontalPos == "from-inside") {
                 return false; // not posible to do it with koffice:anchor-type
             } else if (horizontalPos == "from-left") {
                 d->horizontalAlignment = Left;
             } else if (horizontalPos == "inside") { //svg:x attribute is ignored
                 return false; // not posible to do it with koffice:anchor-type
             } else if (horizontalPos == "left") { //svg:x attribute is ignored
                 d->horizontalAlignment = Left;
                 d->distance.setX(0);
             } else if (horizontalPos == "outside") { //svg:x attribute is ignored
                 return false; // not posible to do it with koffice:anchor-type
             } else if (horizontalPos == "right") { //svg:x attribute is ignored
                 d->horizontalAlignment = Right;
                 d->distance.setX(0);
             }
         } else if (horizontalRel == "paragraph") {
             if (horizontalPos == "center") { //svg:x attribute is ignored
                 d->horizontalAlignment = Center;
                 d->distance.setX(0);
             } else if (horizontalPos == "from-inside") {
                 return false; // not posible to do it with koffice:anchor-type
             } else if (horizontalPos == "from-left") {
                 d->horizontalAlignment = Left;
             } else if (horizontalPos == "inside") { //svg:x attribute is ignored
                 return false; // not posible to do it with koffice:anchor-type
             } else if (horizontalPos == "left") { //svg:x attribute is ignored
                 d->horizontalAlignment = Left;
                 d->distance.setX(0);
             } else if (horizontalPos == "outside") { //svg:x attribute is ignored
                 return false; // not posible to do it with koffice:anchor-type
             } else if (horizontalPos == "right") { //svg:x attribute is ignored
                 d->horizontalAlignment = Right;
                 d->distance.setX(0);
             }
         } else { //TODO another types if needed
             return false;
         }
    }
    else {
        if (anchorType == "paragraph") {
            d->horizontalAlignment = Left;
            d->verticalAlignment = TopOfParagraph;

            // vertical alignment - conversion from style:vertical-rel,pos to koffice:anchor-type
            if (verticalRel == "page") {
                 if (verticalPos == "below") { //svg:y attribute is ignored
                     d->verticalAlignment = BottomOfPage;
                     d->distance.setY(-shape()->size().height());
                 } else if (verticalPos == "bottom") { //svg:y attribute is ignored
                     d->verticalAlignment = BottomOfPage;
                     d->distance.setY(0);
                 } else if (verticalPos == "from-top") {
                     d->verticalAlignment = TopOfPage;
                 } else if (verticalPos == "middle") { //svg:y attribute is ignored
                     return false; // not posible to do it with koffice:anchor-type
                 } else if (verticalPos == "top") { //svg:y attribute is ignored
                     d->verticalAlignment = TopOfPage;
                     d->distance.setY(0);
                 }
             } else if (verticalRel == "page-content") {
                 if (verticalPos == "below") { //svg:y attribute is ignored
                     d->verticalAlignment = BottomOfPageContent;
                     d->distance.setY(-shape()->size().height());
                 } else if (verticalPos == "bottom") { //svg:y attribute is ignored
                     d->verticalAlignment = BottomOfPageContent;
                     d->distance.setY(0);
                 } else if (verticalPos == "from-top") {
                     d->verticalAlignment = TopOfPageContent;
                 } else if (verticalPos == "middle") { //svg:y attribute is ignored
                     return false; // not posible to do it with koffice:anchor-type
                 } else if (verticalPos == "top") { //svg:y attribute is ignored
                     d->verticalAlignment = TopOfPageContent;
                     d->distance.setY(0);
                 }
             } else if (verticalRel == "paragraph") {
                 if (verticalPos == "below") { //svg:y attribute is ignored
                     d->verticalAlignment = BottomOfParagraph;
                     d->distance.setY(-shape()->size().height());
                 } else if (verticalPos == "bottom") { //svg:y attribute is ignored
                     d->verticalAlignment = BottomOfParagraph;
                     d->distance.setY(0);
                 } else if (verticalPos == "from-top") {
                     d->verticalAlignment = TopOfParagraph;
                 } else if (verticalPos == "middle") { //svg:y attribute is ignored
                     return false; // not posible to do it with koffice:anchor-type
                 } else if (verticalPos == "top") { //svg:y attribute is ignored
                     d->verticalAlignment = TopOfParagraph;
                     d->distance.setY(0);
                 }
             } else { //TODO another types if needed
                 return false;
             }

             //horizontal alignment - conversion from style:horizontal-rel,pos to koffice:anchor-type
            if (horizontalRel == "page") {
                 if (horizontalPos == "center") { //svg:x attribute is ignored
                     d->horizontalAlignment = CenterOfPage;
                     d->distance.setX(0);
                 } else if (horizontalPos == "from-inside") {
                     return false; // not posible to do it with koffice:anchor-type
                 } else if (horizontalPos == "from-left") {
                     d->horizontalAlignment = LeftOfPage;
                 } else if (horizontalPos == "inside") { //svg:x attribute is ignored
                     return false; // not posible to do it with koffice:anchor-type
                 } else if (horizontalPos == "left") { //svg:x attribute is ignored
                     d->horizontalAlignment = LeftOfPage;
                     d->distance.setX(0);
                 } else if (horizontalPos == "outside") { //svg:x attribute is ignored
                     return false; // not posible to do it with koffice:anchor-type
                 } else if (horizontalPos == "right") { //svg:x attribute is ignored
                     d->horizontalAlignment = RightOfPage;
                     d->distance.setX(0);
                 }
             } else if (horizontalRel == "page-content") {
                 if (horizontalPos == "center") { //svg:x attribute is ignored
                     return false; // not posible to do it with koffice:anchor-type
                 } else if (horizontalPos == "from-inside") {
                     return false; // not posible to do it with koffice:anchor-type
                 } else if (horizontalPos == "from-left") {
                     d->horizontalAlignment = Left;
                 } else if (horizontalPos == "inside") { //svg:x attribute is ignored
                     return false; // not posible to do it with koffice:anchor-type
                 } else if (horizontalPos == "left") { //svg:x attribute is ignored
                     d->horizontalAlignment = Left;
                     d->distance.setX(0);
                 } else if (horizontalPos == "outside") { //svg:x attribute is ignored
                     return false; // not posible to do it with koffice:anchor-type
                 } else if (horizontalPos == "right") { //svg:x attribute is ignored
                     d->horizontalAlignment = Right;
                     d->distance.setX(0);
                 }
             } else if (horizontalRel == "paragraph") {
                 if (horizontalPos == "center") { //svg:x attribute is ignored
                     d->horizontalAlignment = Center;
                     d->distance.setX(0);
                 } else if (horizontalPos == "from-inside") {
                     return false; // not posible to do it with koffice:anchor-type
                 } else if (horizontalPos == "from-left") {
                     d->horizontalAlignment = Left;
                 } else if (horizontalPos == "inside") { //svg:x attribute is ignored
                     return false; // not posible to do it with koffice:anchor-type
                 } else if (horizontalPos == "left") { //svg:x attribute is ignored
                     d->horizontalAlignment = Left;
                     d->distance.setX(0);
                 } else if (horizontalPos == "outside") { //svg:x attribute is ignored
                     return false; // not posible to do it with koffice:anchor-type
                 } else if (horizontalPos == "right") { //svg:x attribute is ignored
                     d->horizontalAlignment = Right;
                     d->distance.setX(0);
                 }
             } else { //TODO another types if needed
                 return false;
             }
        } else if (anchorType == "frame") {
            d->horizontalAlignment = Left;
            d->verticalAlignment = TopOfFrame;
        }
    }
    return true;
}

bool KTextAnchor::isPositionedInline() const
{
    Q_D(const KTextAnchor);
    return d->isPositionedInline;
}

void KTextAnchor::detachFromModel()
{
    Q_D(KTextAnchor);
    d->model = 0;
}
