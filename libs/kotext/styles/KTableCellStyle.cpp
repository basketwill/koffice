/* This file is part of the KDE project
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Roopesh Chander <roop@forwardbias.in>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2009 KO GmbH <cbo@kogmbh.com>
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
#include "KTableCellStyle.h"
#include "KTableCellStyle_p.h"
#include "KStyleManager.h"
#include <KOdfGenericStyle.h>
#include <KOdfGenericStyles.h>
#include "Styles_p.h"
#include "KTextDocument.h"

#include <KDebug>

#include <QTextTable>
#include <QTextTableFormat>

#include <KUnit.h>
#include <KOdfStyleStack.h>
#include <KOdfLoadingContext.h>
#include <KOdfXmlNS.h>
#include <KXmlWriter.h>

KTableCellStylePrivate::KTableCellStylePrivate()
    : parentStyle(0)
    , next(0)
{
}

KTableCellStylePrivate::~KTableCellStylePrivate()
{
}

void KTableCellStylePrivate::setProperty(int key, const QVariant &value)
{
    stylesPrivate.add(key, value);
}

KTableCellStyle::KTableCellStyle(QObject *parent)
    : KTableBorderStyle(*new KTableCellStylePrivate(), parent)
{
}

KTableCellStyle::KTableCellStyle(const QTextTableCellFormat &format, QObject *parent)
    : KTableBorderStyle(*new KTableCellStylePrivate(), format, parent)
{
    Q_D(KTableCellStyle);
    d->stylesPrivate = format.properties();
}

KTableCellStyle::~KTableCellStyle()
{
}

KTableCellStyle *KTableCellStyle::fromTableCell(const QTextTableCell &tableCell, QObject *parent)
{
    QTextTableCellFormat tableCellFormat = tableCell.format().toTableCellFormat();
    return new KTableCellStyle(tableCellFormat, parent);
}

QRectF KTableCellStyle::contentRect(const QRectF &boundingRect) const
{
    Q_D(const KTableCellStyle);
    return boundingRect.adjusted(
                d->edges[Left].outerPen.widthF() + d->edges[Left].spacing + d->edges[Left].innerPen.widthF() + propertyDouble(QTextFormat::TableCellLeftPadding),
                d->edges[Top].outerPen.widthF() + d->edges[Top].spacing + d->edges[Top].innerPen.widthF() + propertyDouble(QTextFormat::TableCellTopPadding),
                - d->edges[Right].outerPen.widthF() - d->edges[Right].spacing - d->edges[Right].innerPen.widthF() - propertyDouble(QTextFormat::TableCellRightPadding),
                - d->edges[Bottom].outerPen.widthF() - d->edges[Bottom].spacing - d->edges[Bottom].innerPen.widthF() - propertyDouble(QTextFormat::TableCellBottomPadding)
   );
}

QRectF KTableCellStyle::boundingRect(const QRectF &contentRect) const
{
    Q_D(const KTableCellStyle);
    return contentRect.adjusted(
                - d->edges[Left].outerPen.widthF() - d->edges[Left].spacing - d->edges[Left].innerPen.widthF() - propertyDouble(QTextFormat::TableCellLeftPadding),
                - d->edges[Top].outerPen.widthF() - d->edges[Top].spacing - d->edges[Top].innerPen.widthF() - propertyDouble(QTextFormat::TableCellTopPadding),
                d->edges[Right].outerPen.widthF() + d->edges[Right].spacing + d->edges[Right].innerPen.widthF() + propertyDouble(QTextFormat::TableCellRightPadding),
                d->edges[Bottom].outerPen.widthF() + d->edges[Bottom].spacing + d->edges[Bottom].innerPen.widthF() + propertyDouble(QTextFormat::TableCellBottomPadding)
   );
}

void KTableCellStyle::paintBackground(QPainter &painter, const QRectF &bounds) const
{
    QRectF innerBounds = bounds;

    if (hasProperty(CellBackgroundBrush)) {
        painter.fillRect(bounds, background());
    }
}

KTableCellStyle::BorderStyle KTableCellStyle::oasisBorderStyle(const QString &borderstyle)
{
    if (borderstyle == "none")
        return BorderNone;
    if (borderstyle == "double")
        return BorderDouble;
    if (borderstyle == "dotted")
        return BorderDotted;
    if (borderstyle == "dashed")
        return BorderDashed;
    if (borderstyle == "dash-largegap")
        return BorderDashedLong;
    if (borderstyle == "dot-dash") // not offficially odf, but we suppport it anyway
        return BorderDashDot;
    if (borderstyle == "dot-dot-dash") // not offficially odf, but we suppport it anyway
        return BorderDashDotDot;
    if (borderstyle == "slash") // not offficially odf, but we suppport it anyway
        return BorderSlash;
    if (borderstyle == "wave") // not offficially odf, but we suppport it anyway
        return BorderWave;
    if (borderstyle == "double-wave") // not offficially odf, but we suppport it anyway
        return BorderDoubleWave;
    return BorderSolid; // not needed to handle "solid" since it's the default
}

QString KTableCellStyle::odfBorderStyleString(const KTableCellStyle::BorderStyle borderstyle)
{
    switch (borderstyle) {
    case BorderDouble:
        return QString("double");
    case BorderSolid:
        return QString("solid");
    case BorderDashed:
        return QString("dashed");
    case BorderDotted:
        return QString("dotted");
    default:
    case BorderNone:
        return QString("none");
    }
}

void KTableCellStyle::setParentStyle(KTableCellStyle *parent)
{
    Q_D(KTableCellStyle);
    d->parentStyle = parent;
}

void KTableCellStyle::setLeftPadding(qreal padding)
{
    setProperty(QTextFormat::TableCellLeftPadding, padding);
}

void KTableCellStyle::setTopPadding(qreal padding)
{
    setProperty(QTextFormat::TableCellTopPadding, padding);
}

void KTableCellStyle::setRightPadding(qreal padding)
{
    setProperty(QTextFormat::TableCellRightPadding, padding);
}

void KTableCellStyle::setBottomPadding(qreal padding)
{
    setProperty(QTextFormat::TableCellBottomPadding, padding);
}

qreal KTableCellStyle::leftPadding() const
{
    return propertyDouble(QTextFormat::TableCellLeftPadding);
}

qreal KTableCellStyle::rightPadding() const
{
    return propertyDouble(QTextFormat::TableCellRightPadding);
}

qreal KTableCellStyle::topPadding() const
{
    return propertyDouble(QTextFormat::TableCellTopPadding);
}

qreal KTableCellStyle::bottomPadding() const
{
    return propertyDouble(QTextFormat::TableCellBottomPadding);
}

void KTableCellStyle::setPadding(qreal padding)
{
    setBottomPadding(padding);
    setTopPadding(padding);
    setRightPadding(padding);
    setLeftPadding(padding);
}

void KTableCellStyle::setProperty(int key, const QVariant &value)
{
    Q_D(KTableCellStyle);
    if (d->parentStyle) {
        QVariant var = d->parentStyle->value(key);
        if (!var.isNull() && var == value) { // same as parent, so its actually a reset.
            d->stylesPrivate.remove(key);
            return;
        }
    }
    d->stylesPrivate.add(key, value);
}

void KTableCellStyle::remove(int key)
{
    Q_D(KTableCellStyle);
    d->stylesPrivate.remove(key);
}

QVariant KTableCellStyle::value(int key) const
{
    Q_D(const KTableCellStyle);
    QVariant var = d->stylesPrivate.value(key);
    if (var.isNull() && d->parentStyle)
        var = d->parentStyle->value(key);
    return var;
}

bool KTableCellStyle::hasProperty(int key) const
{
    Q_D(const KTableCellStyle);
    return d->stylesPrivate.contains(key);
}

qreal KTableCellStyle::propertyDouble(int key) const
{
    QVariant variant = value(key);
    if (variant.isNull())
        return 0.0;
    return variant.toDouble();
}

int KTableCellStyle::propertyInt(int key) const
{
    QVariant variant = value(key);
    if (variant.isNull())
        return 0;
    return variant.toInt();
}

bool KTableCellStyle::propertyBoolean(int key) const
{
    QVariant variant = value(key);
    if (variant.isNull())
        return false;
    return variant.toBool();
}

QColor KTableCellStyle::propertyColor(int key) const
{
    QVariant variant = value(key);
    if (variant.isNull()) {
        return QColor();
    }
    return qvariant_cast<QColor>(variant);
}

void KTableCellStyle::applyStyle(QTextTableCellFormat &format) const
{
    Q_D(const KTableCellStyle);
    if (d->parentStyle) {
        d->parentStyle->applyStyle(format);
    }
    QList<int> keys = d->stylesPrivate.keys();
    for (int i = 0; i < keys.count(); i++) {
        QVariant variant = d->stylesPrivate.value(keys[i]);
        format.setProperty(keys[i], variant);
    }

    format.setProperty(TopBorderOuterPen, d->edges[Top].outerPen);
    format.setProperty(TopBorderSpacing,  d->edges[Top].spacing);
    format.setProperty(TopBorderInnerPen, d->edges[Top].innerPen);
    format.setProperty(TopBorderStyle, d->borderstyle[Top]);
    format.setProperty(LeftBorderOuterPen, d->edges[Left].outerPen);
    format.setProperty(LeftBorderSpacing,  d->edges[Left].spacing);
    format.setProperty(LeftBorderInnerPen, d->edges[Left].innerPen);
    format.setProperty(LeftBorderStyle, d->borderstyle[Left]);
    format.setProperty(BottomBorderOuterPen, d->edges[Bottom].outerPen);
    format.setProperty(BottomBorderSpacing,  d->edges[Bottom].spacing);
    format.setProperty(BottomBorderInnerPen, d->edges[Bottom].innerPen);
    format.setProperty(BottomBorderStyle, d->borderstyle[Bottom]);
    format.setProperty(RightBorderOuterPen, d->edges[Right].outerPen);
    format.setProperty(RightBorderSpacing,  d->edges[Right].spacing);
    format.setProperty(RightBorderInnerPen, d->edges[Right].innerPen);
    format.setProperty(RightBorderStyle, d->borderstyle[Right]);
    format.setProperty(TopLeftToBottomRightBorderOuterPen, d->edges[TopLeftToBottomRight].outerPen);
    format.setProperty(TopLeftToBottomRightBorderSpacing,  d->edges[TopLeftToBottomRight].spacing);
    format.setProperty(TopLeftToBottomRightBorderInnerPen, d->edges[TopLeftToBottomRight].innerPen);
    format.setProperty(TopLeftToBottomRightBorderStyle, d->borderstyle[TopLeftToBottomRight]);
    format.setProperty(BottomLeftToTopRightBorderOuterPen, d->edges[BottomLeftToTopRight].outerPen);
    format.setProperty(BottomLeftToTopRightBorderSpacing,  d->edges[BottomLeftToTopRight].spacing);
    format.setProperty(BottomLeftToTopRightBorderInnerPen, d->edges[BottomLeftToTopRight].innerPen);
    format.setProperty(BottomLeftToTopRightBorderStyle, d->borderstyle[BottomLeftToTopRight]);
}

void KTableCellStyle::setBackground(const QBrush &brush)
{
    setProperty(CellBackgroundBrush, brush);
}

void KTableCellStyle::clearBackground()
{
    Q_D(KTableCellStyle);
    d->stylesPrivate.remove(CellBackgroundBrush);
}

QBrush KTableCellStyle::background() const
{
    Q_D(const KTableCellStyle);
    QVariant variant = d->stylesPrivate.value(CellBackgroundBrush);

    if (variant.isNull()) {
        return QBrush();
    }
    return qvariant_cast<QBrush>(variant);
}

void KTableCellStyle::setAlignment(Qt::Alignment alignment)
{
    setProperty(QTextFormat::BlockAlignment, (int) alignment);
}

Qt::Alignment KTableCellStyle::alignment() const
{
    return static_cast<Qt::Alignment>(propertyInt(VerticalAlignment));
}

KTableCellStyle *KTableCellStyle::parentStyle() const
{
    Q_D(const KTableCellStyle);
    return d->parentStyle;
}

QString KTableCellStyle::name() const
{
    Q_D(const KTableCellStyle);
    return d->name;
}

void KTableCellStyle::setName(const QString &name)
{
    Q_D(KTableCellStyle);
    if (name == d->name)
        return;
    d->name = name;
    emit nameChanged(name);
}

int KTableCellStyle::styleId() const
{
    return propertyInt(StyleId);
}

void KTableCellStyle::setStyleId(int id)
{
    Q_D(KTableCellStyle);
    setProperty(StyleId, id); if (d->next == 0) d->next = id;
}

QString KTableCellStyle::masterPageName() const
{
    return value(MasterPageName).toString();
}

void KTableCellStyle::setMasterPageName(const QString &name)
{
    setProperty(MasterPageName, name);
}

void KTableCellStyle::loadOdf(const KXmlElement *element, KOdfLoadingContext &context)
{
    Q_D(KTableCellStyle);
    if (element->hasAttributeNS(KOdfXmlNS::style, "display-name"))
        d->name = element->attributeNS(KOdfXmlNS::style, "display-name", QString());

    if (d->name.isEmpty()) // if no style:display-name is given us the style:name
        d->name = element->attributeNS(KOdfXmlNS::style, "name", QString());

    QString masterPage = element->attributeNS(KOdfXmlNS::style, "master-page-name", QString());
    if (! masterPage.isEmpty()) {
        setMasterPageName(masterPage);
    }
    context.styleStack().save();
    QString family = element->attributeNS(KOdfXmlNS::style, "family", "table-cell");
    context.addStyles(element, family.toLocal8Bit().constData());   // Load all parents - only because we don't support inheritance.

    context.styleStack().setTypeProperties("table-cell");
    loadOdfProperties(context.styleStack());

    context.styleStack().setTypeProperties("graphic");
    loadOdfProperties(context.styleStack());

    context.styleStack().setTypeProperties("paragraph");
    loadOdfProperties(context.styleStack());
    context.styleStack().restore();
}

void KTableCellStyle::loadOdfProperties(KOdfStyleStack &styleStack)
{
    // Padding
    if (styleStack.hasProperty(KOdfXmlNS::fo, "padding-left"))
        setLeftPadding(KUnit::parseValue(styleStack.property(KOdfXmlNS::fo, "padding-left")));
    if (styleStack.hasProperty(KOdfXmlNS::fo, "padding-right"))
        setRightPadding(KUnit::parseValue(styleStack.property(KOdfXmlNS::fo, "padding-right")));
    if (styleStack.hasProperty(KOdfXmlNS::fo, "padding-top"))
        setTopPadding(KUnit::parseValue(styleStack.property(KOdfXmlNS::fo, "padding-top")));
    if (styleStack.hasProperty(KOdfXmlNS::fo, "padding-bottom"))
        setBottomPadding(KUnit::parseValue(styleStack.property(KOdfXmlNS::fo, "padding-bottom")));
    if (styleStack.hasProperty(KOdfXmlNS::fo, "padding"))
        setPadding(KUnit::parseValue(styleStack.property(KOdfXmlNS::fo, "padding")));

    // Borders
    if (styleStack.hasProperty(KOdfXmlNS::fo, "border", "left")) {
        QString border = styleStack.property(KOdfXmlNS::fo, "border", "left");
        QString style = border.section(' ', 1, 1);
        if (styleStack.hasProperty(KOdfXmlNS::koffice, "specialborder", "left")) {
            style = styleStack.property(KOdfXmlNS::koffice, "specialborder", "left");
        }
        if (!border.isEmpty() && border != "none" && border != "hidden") {
            setEdge(Left, oasisBorderStyle(style), KUnit::parseValue(border.section(' ', 0, 0), 1.0),QColor(border.section(' ', 2, 2)));
        }
    }
    if (styleStack.hasProperty(KOdfXmlNS::fo, "border", "top")) {
        QString border = styleStack.property(KOdfXmlNS::fo, "border", "top");
        QString style = border.section(' ', 1, 1);
        if (styleStack.hasProperty(KOdfXmlNS::koffice, "specialborder", "top")) {
            style = styleStack.property(KOdfXmlNS::koffice, "specialborder", "top");
        }
        if (!border.isEmpty() && border != "none" && border != "hidden") {
            setEdge(Top, oasisBorderStyle(style), KUnit::parseValue(border.section(' ', 0, 0), 1.0),QColor(border.section(' ', 2, 2)));
        }
    }

    if (styleStack.hasProperty(KOdfXmlNS::fo, "border", "right")) {
        QString border = styleStack.property(KOdfXmlNS::fo, "border", "right");
        QString style = border.section(' ', 1, 1);
        if (styleStack.hasProperty(KOdfXmlNS::koffice, "specialborder", "right")) {
            style = styleStack.property(KOdfXmlNS::koffice, "specialborder", "right");
        }
        if (!border.isEmpty() && border != "none" && border != "hidden") {
            setEdge(Right, oasisBorderStyle(style), KUnit::parseValue(border.section(' ', 0, 0), 1.0),QColor(border.section(' ', 2, 2)));
        }
    }
    if (styleStack.hasProperty(KOdfXmlNS::fo, "border", "bottom")) {
        QString border = styleStack.property(KOdfXmlNS::fo, "border", "bottom");
        QString style = border.section(' ', 1, 1);
        if (styleStack.hasProperty(KOdfXmlNS::koffice, "specialborder", "bottom")) {
            style = styleStack.property(KOdfXmlNS::koffice, "specialborder", "bottom");
        }
        if (!border.isEmpty() && border != "none" && border != "hidden") {
            setEdge(Bottom, oasisBorderStyle(style), KUnit::parseValue(border.section(' ', 0, 0), 1.0),QColor(border.section(' ', 2, 2)));
        }
    }
    if (styleStack.hasProperty(KOdfXmlNS::style, "diagonal-tl-br")) {
        QString border = styleStack.property(KOdfXmlNS::style, "diagonal-tl-br");
        QString style = border.section(' ', 1, 1);
        if (styleStack.hasProperty(KOdfXmlNS::koffice, "specialborder", "tl-br")) {
            style = styleStack.property(KOdfXmlNS::koffice, "specialborder", "tl-br");
        }
        setEdge(TopLeftToBottomRight, oasisBorderStyle(style), KUnit::parseValue(border.section(' ', 0, 0), 1.0),QColor(border.section(' ', 2, 2)));
    }
    if (styleStack.hasProperty(KOdfXmlNS::style, "diagonal-bl-tr")) {
        QString border = styleStack.property(KOdfXmlNS::style, "diagonal-bl-tr");
        QString style = border.section(' ', 1, 1);
        if (styleStack.hasProperty(KOdfXmlNS::koffice, "specialborder", "bl-tr")) {
            style = styleStack.property(KOdfXmlNS::koffice, "specialborder", "bl-tr");
        }
        setEdge(BottomLeftToTopRight, oasisBorderStyle(style), KUnit::parseValue(border.section(' ', 0, 0), 1.0),QColor(border.section(' ', 2, 2)));
    }

    if (styleStack.hasProperty(KOdfXmlNS::style, "border-line-width", "left")) {
        QString borderLineWidth = styleStack.property(KOdfXmlNS::style, "border-line-width", "left");
        if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
            QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
            setEdgeDoubleBorderValues(Left, KUnit::parseValue(blw[0], 1.0), KUnit::parseValue(blw[1], 0.1));
        }
    }
    if (styleStack.hasProperty(KOdfXmlNS::style, "border-line-width", "top")) {
        QString borderLineWidth = styleStack.property(KOdfXmlNS::style, "border-line-width", "top");
        if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
            QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
            setEdgeDoubleBorderValues(Top, KUnit::parseValue(blw[0], 1.0), KUnit::parseValue(blw[1], 0.1));
        }
    }
    if (styleStack.hasProperty(KOdfXmlNS::style, "border-line-width", "right")) {
        QString borderLineWidth = styleStack.property(KOdfXmlNS::style, "border-line-width", "right");
        if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
            QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
            setEdgeDoubleBorderValues(Right, KUnit::parseValue(blw[0], 1.0), KUnit::parseValue(blw[1], 0.1));
        }
    }
    if (styleStack.hasProperty(KOdfXmlNS::style, "border-line-width", "bottom")) {
        QString borderLineWidth = styleStack.property(KOdfXmlNS::style, "border-line-width", "bottom");
        if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
            QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
            setEdgeDoubleBorderValues(Bottom, KUnit::parseValue(blw[0], 1.0), KUnit::parseValue(blw[1], 0.1));
        }
    }
    if (styleStack.hasProperty(KOdfXmlNS::style, "diagonal-tl-br-widths")) {
        QString borderLineWidth = styleStack.property(KOdfXmlNS::style, "diagonal-tl-br-widths");
        if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
            QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
            setEdgeDoubleBorderValues(TopLeftToBottomRight, KUnit::parseValue(blw[0], 1.0), KUnit::parseValue(blw[1], 0.1));
        }
    }
    if (styleStack.hasProperty(KOdfXmlNS::style, "diagonal-bl-tr-widths")) {
        QString borderLineWidth = styleStack.property(KOdfXmlNS::style, "diagonal-bl-tr-widths");
        if (!borderLineWidth.isEmpty() && borderLineWidth != "none" && borderLineWidth != "hidden") {
            QStringList blw = borderLineWidth.split(' ', QString::SkipEmptyParts);
            setEdgeDoubleBorderValues(BottomLeftToTopRight, KUnit::parseValue(blw[0], 1.0), KUnit::parseValue(blw[1], 0.1));
        }
    }

    // The fo:background-color attribute specifies the background color of a cell.
    if (styleStack.hasProperty(KOdfXmlNS::fo, "background-color")) {
        const QString bgcolor = styleStack.property(KOdfXmlNS::fo, "background-color");
        QBrush brush = background();
        if (bgcolor == "transparent")
           clearBackground();
        else {
            if (brush.style() == Qt::NoBrush)
                brush.setStyle(Qt::SolidPattern);
            brush.setColor(bgcolor); // #rrggbb format
            setBackground(brush);
        }
    }

    // Alignment
    const QString verticalAlign(styleStack.property(KOdfXmlNS::style, "vertical-align"));
    if (!verticalAlign.isEmpty()) {
        setAlignment(KOdfText::valignmentFromString(verticalAlign));
    }
}

void KTableCellStyle::copyProperties(const KTableCellStyle *style)
{
    Q_D(KTableCellStyle);
    const KTableCellStylePrivate *styleD = static_cast<const KTableCellStylePrivate*>(style->d_func());

    d->stylesPrivate = styleD->stylesPrivate;
    setName(style->name()); // make sure we emit property change
    d->next = styleD->next;
    d->parentStyle = styleD->parentStyle;
}

KTableCellStyle *KTableCellStyle::clone(QObject *parent)
{
    KTableCellStyle *newStyle = new KTableCellStyle(parent);
    newStyle->copyProperties(this);
    return newStyle;
}


bool KTableCellStyle::operator==(const KTableCellStyle &other) const
{
    Q_D(const KTableCellStyle);
    const KTableCellStylePrivate *otherD = static_cast<const KTableCellStylePrivate*>(other.d_func());
    return otherD->stylesPrivate == d->stylesPrivate;
}

void KTableCellStyle::removeDuplicates(const KTableCellStyle &other)
{
    Q_D(KTableCellStyle);
    const KTableCellStylePrivate *otherD = static_cast<const KTableCellStylePrivate*>(other.d_func());
    d->stylesPrivate.removeDuplicates(otherD->stylesPrivate);
}

void KTableCellStyle::saveOdf(KOdfGenericStyle &style)
{
    Q_UNUSED(style);
/*
    QList<int> keys = d->stylesPrivate.keys();
    foreach(int key, keys) {
        if (key == QTextFormat::BlockAlignment) {
            int alignValue = 0;
            bool ok = false;
            alignValue = d->stylesPrivate.value(key).toInt(&ok);
            if (ok) {
                Qt::Alignment alignment = (Qt::Alignment) alignValue;
                QString align = KOdfText::alignmentToString(alignment);
                if (!align.isEmpty())
                    style.addProperty("fo:text-align", align, KOdfGenericStyle::ParagraphType);
            }
        } else if (key == KTableCellStyle::TextProgressionDirection) {
            int directionValue = 0;
            bool ok = false;
            directionValue = d->stylesPrivate.value(key).toInt(&ok);
            if (ok) {
                QString direction = "";
                if (directionValue == KOdfText::LeftRightTopBottom)
                    direction = "lr";
                else if (directionValue == KOdfText::RightLeftTopBottom)
                    direction = "rl";
                else if (directionValue == KOdfText::TopBottomRightLeft)
                    direction = "tb";
                if (!direction.isEmpty())
                    style.addProperty("style:writing-mode", direction, KOdfGenericStyle::ParagraphType);
            }
        } else if (key == CellBackgroundBrush) {
            QBrush backBrush = background();
            if (backBrush.style() != Qt::NoBrush)
                style.addProperty("fo:background-color", backBrush.color().name(), KOdfGenericStyle::ParagraphType);
            else
                style.addProperty("fo:background-color", "transparent", KOdfGenericStyle::ParagraphType);

    // Border
    QString leftBorder = QString("%1pt %2 %3").arg(QString::number(leftBorderWidth()),
                         odfBorderStyleString(leftBorderStyle()),
                         leftBorderColor().name());
    QString rightBorder = QString("%1pt %2 %3").arg(QString::number(rightBorderWidth()),
                          odfBorderStyleString(rightBorderStyle()),
                          rightBorderColor().name());
    QString topBorder = QString("%1pt %2 %3").arg(QString::number(topBorderWidth()),
                        odfBorderStyleString(topBorderStyle()),
                        topBorderColor().name());
    QString bottomBorder = QString("%1pt %2 %3").arg(QString::number(bottomBorderWidth()),
                           odfBorderStyleString(bottomBorderStyle()),
                           bottomBorderColor().name());
    if (leftBorder == rightBorder && leftBorder == topBorder && leftBorder == bottomBorder) {
        if (leftBorderWidth() > 0 && leftBorderStyle() != KParagraphStyle::BorderNone)
            style.addProperty("fo:border", leftBorder, KOdfGenericStyle::ParagraphType);
    } else {
        if (leftBorderWidth() > 0 && leftBorderStyle() != KParagraphStyle::BorderNone)
            style.addProperty("fo:border-left", leftBorder, KOdfGenericStyle::ParagraphType);
        if (rightBorderWidth() > 0 && rightBorderStyle() != KParagraphStyle::BorderNone)
            style.addProperty("fo:border-right", rightBorder, KOdfGenericStyle::ParagraphType);
        if (topBorderWidth() > 0 && topBorderStyle() != KParagraphStyle::BorderNone)
            style.addProperty("fo:border-top", topBorder, KOdfGenericStyle::ParagraphType);
        if (bottomBorderWidth() > 0 && bottomBorderStyle() != KParagraphStyle::BorderNone)
            style.addProperty("fo:border-bottom", bottomBorder, KOdfGenericStyle::ParagraphType);
    }
    QString leftBorderLineWidth = QString("%1pt %2pt %3pt").arg(QString::number(leftInnerBorderWidth()),
                                  QString::number(leftBorderSpacing()),
                                  QString::number(leftBorderWidth()));
    QString rightBorderLineWidth = QString("%1pt %2pt %3pt").arg(QString::number(rightInnerBorderWidth()),
                                   QString::number(rightBorderSpacing()),
                                   QString::number(rightBorderWidth()));
    QString topBorderLineWidth = QString("%1pt %2pt %3pt").arg(QString::number(topInnerBorderWidth()),
                                 QString::number(topBorderSpacing()),
                                 QString::number(topBorderWidth()));
    QString bottomBorderLineWidth = QString("%1pt %2pt %3pt").arg(QString::number(bottomInnerBorderWidth()),
                                    QString::number(bottomBorderSpacing()),
                                    QString::number(bottomBorderWidth()));
    if (leftBorderLineWidth == rightBorderLineWidth &&
            leftBorderLineWidth == topBorderLineWidth &&
            leftBorderLineWidth == bottomBorderLineWidth &&
            leftBorderStyle() == KParagraphStyle::BorderDouble &&
            rightBorderStyle() == KParagraphStyle::BorderDouble &&
            topBorderStyle() == KParagraphStyle::BorderDouble &&
            bottomBorderStyle() == KParagraphStyle::BorderDouble) {
        style.addProperty("style:border-line-width", leftBorderLineWidth, KOdfGenericStyle::ParagraphType);
    } else {
        if (leftBorderStyle() == KParagraphStyle::BorderDouble)
            style.addProperty("style:border-line-width-left", leftBorderLineWidth, KOdfGenericStyle::ParagraphType);
        if (rightBorderStyle() == KParagraphStyle::BorderDouble)
            style.addProperty("style:border-line-width-right", rightBorderLineWidth, KOdfGenericStyle::ParagraphType);
        if (topBorderStyle() == KParagraphStyle::BorderDouble)
            style.addProperty("style:border-line-width-top", topBorderLineWidth, KOdfGenericStyle::ParagraphType);
        if (bottomBorderStyle() == KParagraphStyle::BorderDouble)
            style.addProperty("style:border-line-width-bottom", bottomBorderLineWidth, KOdfGenericStyle::ParagraphType);
    }
*/
}

#include <KTableCellStyle.moc>
