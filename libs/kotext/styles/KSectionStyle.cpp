/* This file is part of the KDE project
 * Copyright (C) 2006-2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Roopesh Chander <roop@forwardbias.in>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2009 KO GmbH <casper.boemann@kogmbh.com>
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
#include "KSectionStyle.h"
#include "KStyleManager.h"
#include <KOdfGenericStyle.h>
#include <KOdfGenericStyles.h>
#include "Styles_p.h"
#include "KTextDocument.h"

#include <KDebug>

#include <QTextFrame>
#include <QTextFrameFormat>
#include <QTextCursor>
#include <QBuffer>

#include <KUnit.h>
#include <KOdfStyleStack.h>
#include <KOdfLoadingContext.h>
#include <KOdfXmlNS.h>
#include <KXmlWriter.h>
#include <KOdfBorders.h>

class KSectionStyle::Private
{
public:
    Private() : parentStyle(0) {}

    ~Private() {
    }

    void setProperty(int key, const QVariant &value) {
        stylesPrivate.add(key, value);
    }
    int propertyInt(int key) const {
        QVariant variant = stylesPrivate.value(key);
        if (variant.isNull())
            return 0;
        return variant.toInt();
    }
    bool propertyBoolean(int key) const {
        QVariant variant = stylesPrivate.value(key);
        if (variant.isNull())
            return false;
        return variant.toBool();
    }
    qreal propertyDouble(int key) const {
        QVariant variant = stylesPrivate.value(key);
        if (variant.isNull())
            return 0.0;
        return variant.toDouble();
    }
    QColor propertyColor(int key) const {
        QVariant variant = stylesPrivate.value(key);
        if (variant.isNull())
            return QColor();
        return variant.value<QColor>();
    }

    QString name;
    KSectionStyle *parentStyle;
    StylePrivate stylesPrivate;
};

KSectionStyle::KSectionStyle(QObject *parent)
        : QObject(parent), d(new Private())
{
}

KSectionStyle::KSectionStyle(const QTextFrameFormat &sectionFormat, QObject *parent)
        : QObject(parent),
        d(new Private())
{
    d->stylesPrivate = sectionFormat.properties();
}

KSectionStyle::~KSectionStyle()
{
    delete d;
}

void KSectionStyle::setParentStyle(KSectionStyle *parent)
{
    d->parentStyle = parent;
}

void KSectionStyle::setProperty(int key, const QVariant &value)
{
    if (d->parentStyle) {
        QVariant var = d->parentStyle->value(key);
        if (!var.isNull() && var == value) { // same as parent, so its actually a reset.
            d->stylesPrivate.remove(key);
            return;
        }
    }
    d->stylesPrivate.add(key, value);
}

void KSectionStyle::remove(int key)
{
    d->stylesPrivate.remove(key);
}

QVariant KSectionStyle::value(int key) const
{
    QVariant var = d->stylesPrivate.value(key);
    if (var.isNull() && d->parentStyle)
        var = d->parentStyle->value(key);
    return var;
}

bool KSectionStyle::hasProperty(int key) const
{
    return d->stylesPrivate.contains(key);
}

void KSectionStyle::applyStyle(QTextFrameFormat &format) const
{
    if (d->parentStyle) {
        d->parentStyle->applyStyle(format);
    }
    QList<int> keys = d->stylesPrivate.keys();
    for (int i = 0; i < keys.count(); i++) {
        QVariant variant = d->stylesPrivate.value(keys[i]);
        format.setProperty(keys[i], variant);
    }
}

void KSectionStyle::applyStyle(QTextFrame &section) const
{
    QTextFrameFormat format = section.frameFormat();
    applyStyle(format);
    section.setFrameFormat(format);
}

void KSectionStyle::unapplyStyle(QTextFrame &section) const
{
    if (d->parentStyle)
        d->parentStyle->unapplyStyle(section);

    QTextFrameFormat format = section.frameFormat();

    QList<int> keys = d->stylesPrivate.keys();
    for (int i = 0; i < keys.count(); i++) {
        QVariant variant = d->stylesPrivate.value(keys[i]);
        if (variant == format.property(keys[i]))
            format.clearProperty(keys[i]);
    }
    section.setFrameFormat(format);
}

void KSectionStyle::setLeftMargin(qreal margin)
{
    setProperty(QTextFormat::BlockLeftMargin, margin);
}

qreal KSectionStyle::leftMargin() const
{
    return d->propertyDouble(QTextFormat::BlockLeftMargin);
}

void KSectionStyle::setRightMargin(qreal margin)
{
    setProperty(QTextFormat::BlockRightMargin, margin);
}

qreal KSectionStyle::rightMargin() const
{
    return d->propertyDouble(QTextFormat::BlockRightMargin);
}

KSectionStyle *KSectionStyle::parentStyle() const
{
    return d->parentStyle;
}

QString KSectionStyle::name() const
{
    return d->name;
}

void KSectionStyle::setName(const QString &name)
{
    if (name == d->name)
        return;
    d->name = name;
    emit nameChanged(name);
}

int KSectionStyle::styleId() const
{
    return d->propertyInt(StyleId);
}

void KSectionStyle::setStyleId(int id)
{
    setProperty(StyleId, id);
}


KOdfText::Direction KSectionStyle::textProgressionDirection() const
{
    return static_cast<KOdfText::Direction>(d->propertyInt(TextProgressionDirection));
}

void KSectionStyle::setTextProgressionDirection(KOdfText::Direction dir)
{
    setProperty(TextProgressionDirection, dir);
}

void KSectionStyle::setBackground(const QBrush &brush)
{
    d->setProperty(QTextFormat::BackgroundBrush, brush);
}

void KSectionStyle::clearBackground()
{
    d->stylesPrivate.remove(QTextCharFormat::BackgroundBrush);
}

QBrush KSectionStyle::background() const
{
    QVariant variant = d->stylesPrivate.value(QTextFormat::BackgroundBrush);

    if (variant.isNull()) {
        QBrush brush;
        return brush;
    }
    return qvariant_cast<QBrush>(variant);
}

void KSectionStyle::loadOdf(const KXmlElement *element, KOdfLoadingContext &context)
{
    if (element->hasAttributeNS(KOdfXmlNS::style, "display-name"))
        d->name = element->attributeNS(KOdfXmlNS::style, "display-name", QString());

    if (d->name.isEmpty()) // if no style:display-name is given us the style:name
        d->name = element->attributeNS(KOdfXmlNS::style, "name", QString());

    context.styleStack().save();
    // Load all parents - only because we don't support inheritance.
    QString family = element->attributeNS(KOdfXmlNS::style, "family", "section");
    context.addStyles(element, family.toLocal8Bit().constData());   // Load all parents - only because we don't support inheritance.

    context.styleStack().setTypeProperties("section");   // load all style attributes from "style:section-properties"

    KOdfStyleStack &styleStack = context.styleStack();

    // in 1.6 this was defined at KoParagLayout::loadOasisParagLayout(KoParagLayout&, KoOasisContext&)

    if (styleStack.hasProperty(KOdfXmlNS::style, "writing-mode")) {     // http://www.w3.org/TR/2004/WD-xsl11-20041216/#writing-mode
        QString writingMode = styleStack.property(KOdfXmlNS::style, "writing-mode");
        setTextProgressionDirection(KOdfText::directionFromString(writingMode));
    }

    // Indentation (margin)
    bool hasMarginLeft = styleStack.hasProperty(KOdfXmlNS::fo, "margin-left");
    bool hasMarginRight = styleStack.hasProperty(KOdfXmlNS::fo, "margin-right");
    if (hasMarginLeft)
        setLeftMargin(KUnit::parseValue(styleStack.property(KOdfXmlNS::fo, "margin-left")));
    if (hasMarginRight)
        setRightMargin(KUnit::parseValue(styleStack.property(KOdfXmlNS::fo, "margin-right")));


    // The fo:background-color attribute specifies the background color of a paragraph.
    if (styleStack.hasProperty(KOdfXmlNS::fo, "background-color")) {
        const QString bgcolor = styleStack.property(KOdfXmlNS::fo, "background-color");
        QBrush brush = background();
        if (bgcolor == "transparent")
            brush.setStyle(Qt::NoBrush);
        else {
            if (brush.style() == Qt::NoBrush)
                brush.setStyle(Qt::SolidPattern);
            brush.setColor(bgcolor); // #rrggbb format
        }
        setBackground(brush);
    }

    styleStack.restore();
}


void KSectionStyle::copyProperties(const KSectionStyle *style)
{
    d->stylesPrivate = style->d->stylesPrivate;
    setName(style->name()); // make sure we emit property change
    d->parentStyle = style->d->parentStyle;
}

KSectionStyle *KSectionStyle::clone(QObject *parent) const
{
    KSectionStyle *newStyle = new KSectionStyle(parent);
    newStyle->copyProperties(this);
    return newStyle;
}

bool KSectionStyle::operator==(const KSectionStyle &other) const
{
    return other.d->stylesPrivate == d->stylesPrivate;
}

void KSectionStyle::removeDuplicates(const KSectionStyle &other)
{
    d->stylesPrivate.removeDuplicates(other.d->stylesPrivate);
}

void KSectionStyle::saveOdf(KOdfGenericStyle &style)
{
    // only custom style have a displayname. automatic styles don't have a name set.
    if (!d->name.isEmpty() && !style.isDefaultStyle()) {
        style.addAttribute("style:display-name", d->name);
    }

    QList<int> keys = d->stylesPrivate.keys();
    foreach(int key, keys) {
        if (key == KSectionStyle::TextProgressionDirection) {
            int directionValue = 0;
            bool ok = false;
            directionValue = d->stylesPrivate.value(key).toInt(&ok);
            if (ok) {
                QString direction;
                if (directionValue == KOdfText::LeftRightTopBottom)
                    direction = "lr-tb";
                else if (directionValue == KOdfText::RightLeftTopBottom)
                    direction = "rl-tb";
                else if (directionValue == KOdfText::TopBottomRightLeft)
                    direction = "tb-lr";
                else if (directionValue == KOdfText::InheritDirection)
                    direction = "page";
                if (!direction.isEmpty())
                    style.addProperty("style:writing-mode", direction, KOdfGenericStyle::DefaultType);
            }
        } else if (key == QTextFormat::BackgroundBrush) {
            QBrush backBrush = background();
            if (backBrush.style() != Qt::NoBrush)
                style.addProperty("fo:background-color", backBrush.color().name(), KOdfGenericStyle::ParagraphType);
            else
                style.addProperty("fo:background-color", "transparent", KOdfGenericStyle::DefaultType);
        } else if (key == QTextFormat::BlockLeftMargin) {
            style.addPropertyPt("fo:margin-left", leftMargin(), KOdfGenericStyle::DefaultType);
        } else if (key == QTextFormat::BlockRightMargin) {
            style.addPropertyPt("fo:margin-right", rightMargin(), KOdfGenericStyle::DefaultType);
      }
    }
}

#include <KSectionStyle.moc>
