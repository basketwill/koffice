/* This file is part of the KDE project
 * Copyright (C) 2006-2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
*  Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
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
#include "KCharacterStyle.h"

#include "Styles_p.h"

#include <QTextBlock>
#include <QTextCursor>
#include <QFontMetrics>

#include <KOdfLoadingContext.h>
#include <KOdfStylesReader.h>
#include <KOdfXmlNS.h>
#include <KXmlReader.h>
#include <KUnit.h>
#include <KOdfGenericStyle.h>
#include <KPostscriptPaintDevice.h>
#include <KShapeLoadingContext.h>
#include "KTextSharedLoadingData.h"

#include <KDebug>

class KCharacterStyle::Private
{
public:
    Private();
    ~Private() { }

    void setProperty(int key, const QVariant &value) {
        stylesPrivate.add(key, value);
    }
    qreal propertyDouble(int key) const {
        QVariant variant = stylesPrivate.value(key);
        if (variant.isNull())
            return 0.0;
        return variant.toDouble();
    }
    int propertyInt(int key) const {
        QVariant variant = stylesPrivate.value(key);
        if (variant.isNull())
            return 0;
        return variant.toInt();
    }
    QString propertyString(int key) const {
        QVariant variant = stylesPrivate.value(key);
        if (variant.isNull())
            return QString();
        return qvariant_cast<QString>(variant);
    }
    bool propertyBoolean(int key) const {
        QVariant variant = stylesPrivate.value(key);
        if (variant.isNull())
            return false;
        return variant.toBool();
    }
    QColor propertyColor(int key) const {
        QVariant variant = stylesPrivate.value(key);
        if (variant.isNull())
            return QColor();
        return variant.value<QColor>();
    }

    QString name;
    StylePrivate stylesPrivate;
};

KCharacterStyle::Private::Private()
{
}

KCharacterStyle::KCharacterStyle(QObject *parent)
        : QObject(parent), d(new Private())
{
}

KCharacterStyle::KCharacterStyle(const QTextCharFormat &format, QObject *parent)
        : QObject(parent), d(new Private())
{
    copyProperties(format);
}

void KCharacterStyle::copyProperties(const KCharacterStyle *style)
{
    d->stylesPrivate = style->d->stylesPrivate;
    setName(style->name()); // make sure we emit property change
}

void KCharacterStyle::copyProperties(const QTextCharFormat &format)
{
    d->stylesPrivate = format.properties();
}

KCharacterStyle *KCharacterStyle::clone(QObject *parent)
{
    KCharacterStyle *newStyle = new KCharacterStyle(parent);
    newStyle->copyProperties(this);
    return newStyle;
}

KCharacterStyle::~KCharacterStyle()
{
    delete d;
}

QPen KCharacterStyle::textOutline() const
{
    QVariant variant = d->stylesPrivate.value(QTextFormat::TextOutline);
    if (variant.isNull()) {
        return QPen(Qt::NoPen);
    }
    return qvariant_cast<QPen>(variant);
}

QBrush KCharacterStyle::background() const
{
    QVariant variant = d->stylesPrivate.value(QTextFormat::BackgroundBrush);

    if (variant.isNull()) {
        return QBrush();
    }
    return qvariant_cast<QBrush>(variant);
}

void KCharacterStyle::clearBackground()
{
    d->stylesPrivate.remove(QTextCharFormat::BackgroundBrush);
}

QBrush KCharacterStyle::foreground() const
{
    QVariant variant = d->stylesPrivate.value(QTextFormat::ForegroundBrush);
    if (variant.isNull()) {
        return QBrush();
    }
    return qvariant_cast<QBrush>(variant);
}

void KCharacterStyle::clearForeground()
{
    d->stylesPrivate.remove(QTextCharFormat::ForegroundBrush);
}

void KCharacterStyle::applyStyle(QTextCharFormat &format) const
{
    const QMap<int, QVariant> props = d->stylesPrivate.properties();
    QMap<int, QVariant>::const_iterator it = props.begin();
    while (it != props.end()) {
        if (!it.value().isNull()) {
            // kDebug() << "[" + name() + "]" << QString::number(it.key(), 3, 16) << it.value();
            format.setProperty(it.key(), it.value());
        }
        ++it;
    }
}

void KCharacterStyle::applyStyle(QTextCursor *selection) const
{
    QTextCharFormat cf;
    applyStyle(cf);
    selection->mergeCharFormat(cf);
}

void KCharacterStyle::unapplyStyle(QTextCharFormat &format) const
{
    QMap<int, QVariant> props = d->stylesPrivate.properties();
    QMap<int, QVariant>::const_iterator it = props.constBegin();
    while (it != props.constEnd()) {
        if (!it.value().isNull() && it.value() == format.property(it.key())) {
           format.clearProperty(it.key());
        }
        ++it;
    }
}

// OASIS 14.2.29
static void importOdfLine(const QString &type, const QString &style, const QString &width,
                          KCharacterStyle::LineStyle &lineStyle, KCharacterStyle::LineType &lineType,
                          KCharacterStyle::LineWeight &lineWeight, qreal &lineWidth)
{
    lineStyle = KCharacterStyle::NoLineStyle;
    lineType = KCharacterStyle::NoLineType;
    lineWidth = 0;
    lineWeight = KCharacterStyle::AutoLineWeight;

    QString fixedType = type;
    QString fixedStyle = style;
    if (fixedStyle == "none")
        fixedType.clear();
    else if (fixedType.isEmpty() && !fixedStyle.isEmpty())
        fixedType = "single";
    else if (!fixedType.isEmpty() && fixedType != "none" && fixedStyle.isEmpty()) {
        // don't set a style when the type is none
        fixedStyle = "solid";
    }

    if (fixedType == "single")
        lineType = KCharacterStyle::SingleLine;
    else if (fixedType == "double")
        lineType = KCharacterStyle::DoubleLine;

    if (fixedStyle == "solid")
        lineStyle = KCharacterStyle::SolidLine;
    else if (fixedStyle == "dotted")
        lineStyle = KCharacterStyle::DottedLine;
    else if (fixedStyle == "dash")
        lineStyle = KCharacterStyle::DashLine;
    else if (fixedStyle == "long-dash")
        lineStyle = KCharacterStyle::LongDashLine;
    else if (fixedStyle == "dot-dash")
        lineStyle = KCharacterStyle::DotDashLine;
    else if (fixedStyle == "dot-dot-dash")
        lineStyle = KCharacterStyle::DotDotDashLine;
    else if (fixedStyle == "wave")
        lineStyle = KCharacterStyle::WaveLine;

    if (width.isEmpty() || width == "auto")
        lineWeight = KCharacterStyle::AutoLineWeight;
    else if (width == "normal")
        lineWeight = KCharacterStyle::NormalLineWeight;
    else if (width == "bold")
        lineWeight = KCharacterStyle::BoldLineWeight;
    else if (width == "thin")
        lineWeight = KCharacterStyle::ThinLineWeight;
    else if (width == "dash")
        lineWeight = KCharacterStyle::DashLineWeight;
    else if (width == "medium")
        lineWeight = KCharacterStyle::MediumLineWeight;
    else if (width == "thick")
        lineWeight = KCharacterStyle::ThickLineWeight;
    else if (width.endsWith('%')) {
        lineWeight = KCharacterStyle::PercentLineWeight;
        lineWidth = width.mid(0, width.length() - 1).toDouble();
    } else if (width[width.length()-1].isNumber()) {
        lineWeight = KCharacterStyle::PercentLineWeight;
        lineWidth = 100 * width.toDouble();
    } else {
        lineWeight = KCharacterStyle::LengthLineWeight;
        lineWidth = KUnit::parseValue(width);
    }
}

static QString exportOdfLineType(KCharacterStyle::LineType lineType)
{
    switch (lineType) {
    case KCharacterStyle::NoLineType:
        return "none";
    case KCharacterStyle::SingleLine:
        return "single";
    case KCharacterStyle::DoubleLine:
        return "double";
    default:
        return "";
    }
}

static QString exportOdfLineStyle(KCharacterStyle::LineStyle lineStyle)
{
    switch (lineStyle) {
    case KCharacterStyle::NoLineStyle:
        return "none";
    case KCharacterStyle::SolidLine:
        return "solid";
    case KCharacterStyle::DottedLine:
        return "dotted";
    case KCharacterStyle::DashLine:
        return "dash";
    case KCharacterStyle::LongDashLine:
        return "long-dash";
    case KCharacterStyle::DotDashLine:
        return "dot-dash";
    case KCharacterStyle::DotDotDashLine:
        return "dot-dot-dash";
    case KCharacterStyle::WaveLine:
        return "wave";
    default:
        return "";
    }
}
static QString exportOdfLineMode(KCharacterStyle::LineMode lineMode)
{
    switch (lineMode) {
    case KCharacterStyle::ContinuousLineMode:
        return "continuous";
    case KCharacterStyle::SkipWhiteSpaceLineMode:
        return "skip-white-space";
    default:
        return "";
    }
}

static QString exportOdfLineWidth(KCharacterStyle::LineWeight lineWeight, qreal lineWidth)
{
    switch (lineWeight) {
    case KCharacterStyle::AutoLineWeight:
        return "auto";
    case KCharacterStyle::NormalLineWeight:
        return "normal";
    case KCharacterStyle::BoldLineWeight:
        return "bold";
    case KCharacterStyle::ThinLineWeight:
        return "thin";
    case KCharacterStyle::DashLineWeight:
        return "dash";
    case KCharacterStyle::MediumLineWeight:
        return "medium";
    case KCharacterStyle::ThickLineWeight:
        return "thick";
    case KCharacterStyle::PercentLineWeight:
        return QString("%1%").arg(lineWidth);
    case KCharacterStyle::LengthLineWeight:
        return QString("%1pt").arg(lineWidth);
    default:
        return QString();
    }
}

static QString exportOdfFontStyleHint(QFont::StyleHint hint)
{
    switch (hint) {
    case QFont::Serif:
        return "roman";
    case QFont::SansSerif:
        return "swiss";
    case QFont::TypeWriter:
        return "modern";
    case QFont::Decorative:
        return "decorative";
    case QFont::System:
        return "system";
        /*case QFont::Script */
    default:
        return "";
    }
}

void KCharacterStyle::setFontFamily(const QString &family)
{
    d->setProperty(QTextFormat::FontFamily, family);
}
QString KCharacterStyle::fontFamily() const
{
    return d->propertyString(QTextFormat::FontFamily);
}
void KCharacterStyle::setFontPointSize(qreal size)
{
    d->setProperty(QTextFormat::FontPointSize, size);
}
qreal KCharacterStyle::fontPointSize() const
{
    return d->propertyDouble(QTextFormat::FontPointSize);
}
void KCharacterStyle::setFontWeight(int weight)
{
    d->setProperty(QTextFormat::FontWeight, weight);
}
int KCharacterStyle::fontWeight() const
{
    return d->propertyInt(QTextFormat::FontWeight);
}
void KCharacterStyle::setFontItalic(bool italic)
{
    d->setProperty(QTextFormat::FontItalic, italic);
}
bool KCharacterStyle::fontItalic() const
{
    return d->propertyBoolean(QTextFormat::FontItalic);
}
void KCharacterStyle::setFontOverline(bool overline)
{
    d->setProperty(QTextFormat::FontOverline, overline);
}
bool KCharacterStyle::fontOverline() const
{
    return d->propertyBoolean(QTextFormat::FontOverline);
}
void KCharacterStyle::setFontFixedPitch(bool fixedPitch)
{
    d->setProperty(QTextFormat::FontFixedPitch, fixedPitch);
}
bool KCharacterStyle::fontFixedPitch() const
{
    return d->propertyBoolean(QTextFormat::FontFixedPitch);
}
void KCharacterStyle::setFontStyleHint(QFont::StyleHint styleHint)
{
    d->setProperty(QTextFormat::FontStyleHint, styleHint);
}
QFont::StyleHint KCharacterStyle::fontStyleHint() const
{
    return static_cast<QFont::StyleHint>(d->propertyInt(QTextFormat::FontStyleHint));
}
void KCharacterStyle::setFontKerning(bool enable)
{
    d->setProperty(QTextFormat::FontKerning, enable);
}
bool KCharacterStyle::fontKerning() const
{
    return d->propertyBoolean(QTextFormat::FontKerning);
}
void KCharacterStyle::setVerticalAlignment(QTextCharFormat::VerticalAlignment alignment)
{
    d->setProperty(QTextFormat::TextVerticalAlignment, alignment);
}
QTextCharFormat::VerticalAlignment KCharacterStyle::verticalAlignment() const
{
    return static_cast<QTextCharFormat::VerticalAlignment>(d->propertyInt(QTextFormat::TextVerticalAlignment));
}
void KCharacterStyle::setTextOutline(const QPen &pen)
{
    d->setProperty(QTextFormat::TextOutline, pen);
}
void KCharacterStyle::setBackground(const QBrush &brush)
{
    d->setProperty(QTextFormat::BackgroundBrush, brush);
}
void KCharacterStyle::setForeground(const QBrush &brush)
{
    d->setProperty(QTextFormat::ForegroundBrush, brush);
}
QString KCharacterStyle::name() const
{
    return d->name;
}
void KCharacterStyle::setName(const QString &name)
{
    if (name == d->name)
        return;
    d->name = name;
    emit nameChanged(name);
}
int KCharacterStyle::styleId() const
{
    return d->propertyInt(StyleId);
}
void KCharacterStyle::setStyleId(int id)
{
    d->setProperty(StyleId, id);
}
QFont KCharacterStyle::font() const
{
    QFont font;
    if (d->stylesPrivate.contains(QTextFormat::FontFamily))
        font.setFamily(fontFamily());
    if (d->stylesPrivate.contains(QTextFormat::FontPointSize))
        font.setPointSizeF(fontPointSize());
    if (d->stylesPrivate.contains(QTextFormat::FontWeight))
        font.setWeight(fontWeight());
    if (d->stylesPrivate.contains(QTextFormat::FontItalic))
        font.setItalic(fontItalic());
    return font;
}
void KCharacterStyle::setHasHyphenation(bool on)
{
    d->setProperty(HasHyphenation, on);
}
bool KCharacterStyle::hasHyphenation() const
{
    return d->propertyBoolean(HasHyphenation);
}
void KCharacterStyle::setStrikeOutStyle(KCharacterStyle::LineStyle strikeOut)
{
    d->setProperty(StrikeOutStyle, strikeOut);
}

KCharacterStyle::LineStyle KCharacterStyle::strikeOutStyle() const
{
    return (KCharacterStyle::LineStyle) d->propertyInt(StrikeOutStyle);
}

void KCharacterStyle::setStrikeOutType(LineType lineType)
{
    d->setProperty(StrikeOutType, lineType);
}

KCharacterStyle::LineType KCharacterStyle::strikeOutType() const
{
    return (KCharacterStyle::LineType) d->propertyInt(StrikeOutType);
}

void KCharacterStyle::setStrikeOutColor(const QColor &color)
{
    d->setProperty(StrikeOutColor, color);
}

QColor KCharacterStyle::strikeOutColor() const
{
    return d->propertyColor(StrikeOutColor);
}

void KCharacterStyle::setStrikeOutWidth(LineWeight weight, qreal width)
{
    d->setProperty(KCharacterStyle::StrikeOutWeight, weight);
    d->setProperty(KCharacterStyle::StrikeOutWidth, width);
}

void KCharacterStyle::strikeOutWidth(LineWeight &weight, qreal &width) const
{
    weight = (KCharacterStyle::LineWeight) d->propertyInt(KCharacterStyle::StrikeOutWeight);
    width = d->propertyDouble(KCharacterStyle::StrikeOutWidth);
}
void KCharacterStyle::setStrikeOutMode(LineMode lineMode)
{
    d->setProperty(StrikeOutMode, lineMode);
}

void KCharacterStyle::setStrikeOutText(const QString &text)
{
    d->setProperty(StrikeOutText, text);
}
QString KCharacterStyle::strikeOutText() const
{
    return d->propertyString(StrikeOutText);
}
KCharacterStyle::LineMode KCharacterStyle::strikeOutMode() const
{
    return (KCharacterStyle::LineMode) d->propertyInt(StrikeOutMode);
}

void KCharacterStyle::setUnderlineStyle(KCharacterStyle::LineStyle underline)
{
    d->setProperty(UnderlineStyle, underline);
}

KCharacterStyle::LineStyle KCharacterStyle::underlineStyle() const
{
    return (KCharacterStyle::LineStyle) d->propertyInt(UnderlineStyle);
}

void KCharacterStyle::setUnderlineType(LineType lineType)
{
    d->setProperty(UnderlineType, lineType);
}

KCharacterStyle::LineType KCharacterStyle::underlineType() const
{
    return (KCharacterStyle::LineType) d->propertyInt(UnderlineType);
}

void KCharacterStyle::setUnderlineColor(const QColor &color)
{
    d->setProperty(QTextFormat::TextUnderlineColor, color);
}

QColor KCharacterStyle::underlineColor() const
{
    return d->propertyColor(QTextFormat::TextUnderlineColor);
}

void KCharacterStyle::setUnderlineWidth(LineWeight weight, qreal width)
{
    d->setProperty(KCharacterStyle::UnderlineWeight, weight);
    d->setProperty(KCharacterStyle::UnderlineWidth, width);
}

void KCharacterStyle::underlineWidth(LineWeight &weight, qreal &width) const
{
    weight = (KCharacterStyle::LineWeight) d->propertyInt(KCharacterStyle::UnderlineWeight);
    width = d->propertyDouble(KCharacterStyle::UnderlineWidth);
}

void KCharacterStyle::setUnderlineMode(LineMode mode)
{
    d->setProperty(KCharacterStyle::UnderlineMode, mode);
}

KCharacterStyle::LineMode KCharacterStyle::underlineMode() const
{
    return static_cast<KCharacterStyle::LineMode>(d->propertyInt(KCharacterStyle::UnderlineMode));
}

void KCharacterStyle::setFontLetterSpacing(qreal spacing)
{
    d->setProperty(QTextCharFormat::FontLetterSpacing, spacing);
}

qreal KCharacterStyle::fontLetterSpacing() const
{
    return d->propertyDouble(QTextCharFormat::FontLetterSpacing);
}

void KCharacterStyle::setFontWordSpacing(qreal spacing)
{
    d->setProperty(QTextCharFormat::FontWordSpacing, spacing);
}

qreal KCharacterStyle::fontWordSpacing() const
{
    return d->propertyDouble(QTextCharFormat::FontWordSpacing);
}


void KCharacterStyle::setFontCapitalization(QFont::Capitalization capitalization)
{
    d->setProperty(QTextFormat::FontCapitalization, capitalization);
}

QFont::Capitalization KCharacterStyle::fontCapitalization() const
{
    return (QFont::Capitalization) d->propertyInt(QTextFormat::FontCapitalization);
}

void KCharacterStyle::setCountry(const QString &country)
{
    if (country.isEmpty())
        d->stylesPrivate.remove(KCharacterStyle::Country);
    else
        d->setProperty(KCharacterStyle::Country, country);
}

void KCharacterStyle::setLanguage(const QString &language)
{
    if (language.isEmpty())
        d->stylesPrivate.remove(KCharacterStyle::Language);
    else
        d->setProperty(KCharacterStyle::Language, language);
}

QString KCharacterStyle::country() const
{
    return d->stylesPrivate.value(KCharacterStyle::Country).toString();
}

QString KCharacterStyle::language() const
{
    return d->propertyString(KCharacterStyle::Language);
}

bool KCharacterStyle::hasProperty(int key) const
{
    return d->stylesPrivate.contains(key);
}

static KCharacterStyle::RotationAngle intToRotationAngle(int angle)
{
    KCharacterStyle::RotationAngle rotationAngle = KCharacterStyle::Zero;
    if (angle == 90) {
        rotationAngle = KCharacterStyle::Ninety;
    } else if (angle == 270) {
        rotationAngle = KCharacterStyle::TwoHundredSeventy;
    }
    return rotationAngle;
}

static int rotationAngleToInt(KCharacterStyle::RotationAngle rotationAngle)
{
    int angle = 0;
    if (rotationAngle == KCharacterStyle::Ninety) {
        angle = 90;
    } else if (rotationAngle == KCharacterStyle::TwoHundredSeventy) {
        angle = 270;
    }
    return angle;
}

static QString rotationScaleToString(KCharacterStyle::RotationScale rotationScale)
{
    QString scale = "line-height";
    if (rotationScale == KCharacterStyle::Fixed) {
        scale = "fixed";
    }
    return scale;
}

static KCharacterStyle::RotationScale stringToRotationScale(const QString &scale)
{
    KCharacterStyle::RotationScale rotationScale = KCharacterStyle::LineHeight;
    if (scale == "fixed") {
        rotationScale = KCharacterStyle::Fixed;
    }
    return rotationScale;
}

void KCharacterStyle::setTextRotationAngle(RotationAngle angle)
{
    d->setProperty(TextRotationAngle, rotationAngleToInt(angle));
}

KCharacterStyle::RotationAngle KCharacterStyle::textRotationAngle() const
{
    return intToRotationAngle(d->propertyInt(TextRotationAngle));
}

void KCharacterStyle::setTextRotationScale(RotationScale scale)
{
    d->setProperty(TextRotationScale, rotationScaleToString(scale));
}

KCharacterStyle::RotationScale KCharacterStyle::textRotationScale() const
{
    return stringToRotationScale(d->propertyString(TextRotationScale));
}

void KCharacterStyle::setTextScale(int scale)
{
    d->setProperty(TextScale, scale);
}

int KCharacterStyle::textScale() const
{
    return d->propertyInt(TextScale);
}

//in 1.6 this was defined in KoTextFormat::load(KoOasisContext &context)
void KCharacterStyle::loadOdf(KShapeLoadingContext &scontext)
{
    KOdfLoadingContext &context = scontext.odfLoadingContext();
    KOdfStyleStack &styleStack = context.styleStack();
    loadOdfProperties(styleStack);

    QString fontName;
    if (styleStack.hasProperty(KOdfXmlNS::style, "font-name")) {
        // This font name is a reference to a font face declaration.
        KOdfStylesReader &stylesReader = context.stylesReader();
        const KXmlElement *fontFace = stylesReader.findStyle(styleStack.property(KOdfXmlNS::style, "font-name"));
        if (fontFace != 0)
            fontName = fontFace->attributeNS(KOdfXmlNS::svg, "font-family", "");
    }
    if (! fontName.isEmpty()) {
        // Hmm, the remove "'" could break it's in the middle of the fontname...
        fontName = fontName.remove('\'');

        // 'Thorndale' is not known outside OpenOffice so we substitute it
        // with 'Times New Roman' that looks nearly the same.
        if (fontName == "Thorndale") {
            fontName = "Times New Roman";
        } else {
		int name_length = fontName.length() - 3;
		if (name_length > 0 && fontName.endsWith("CE")
                && fontName[name_length].isSpace()) {
            		fontName.chop(3);
		}
        }
        setFontFamily(fontName);
    }
}

void KCharacterStyle::loadOdfProperties(KOdfStyleStack &styleStack)
{
    // The fo:color attribute specifies the foreground color of text.
    const QString color(styleStack.property(KOdfXmlNS::fo, "color"));
    if (!color.isEmpty()) {
        QColor c(color);
        if (c.isValid()) {     // 3.10.3
            setForeground(QBrush(c));
        }
        // if (styleStack.property(KOdfXmlNS::style, "use-window-font-color") == "true")
            // we should store this property to allow the layout to ignore the above set color in some situations.
    }

    QString fontName(styleStack.property(KOdfXmlNS::fo, "font-family"));
    if (!fontName.isEmpty()) {
        // Specify whether a font has a fixed or variable width.
        // These attributes are ignored if there is no corresponding fo:font-family attribute attached to the same formatting properties element.
        const QString fontPitch(styleStack.property(KOdfXmlNS::style, "font-pitch"));
        if (!fontPitch.isEmpty()) {
            setFontFixedPitch(fontPitch == "fixed");
        }

        const QString genericFamily(styleStack.property(KOdfXmlNS::style, "font-family-generic"));
        if (!genericFamily.isEmpty()) {
            if (genericFamily == "roman")
                setFontStyleHint(QFont::Serif);
            else if (genericFamily == "swiss")
                setFontStyleHint(QFont::SansSerif);
            else if (genericFamily == "modern")
                setFontStyleHint(QFont::TypeWriter);
            else if (genericFamily == "decorative")
                setFontStyleHint(QFont::Decorative);
            else if (genericFamily == "system")
                setFontStyleHint(QFont::System);
            else if (genericFamily == "script") {
                ; // TODO: no hint available in Qt yet, we should at least store it as a property internally!
            }
        }

        const QString fontCharset(styleStack.property(KOdfXmlNS::style, "font-charset"));
        if (!fontCharset.isEmpty()) {
            // this property is not required by Qt, since Qt auto selects the right font based on the text
            // The only charset of interest to us is x-symbol - this should disable spell checking
            d->setProperty(KCharacterStyle::FontCharset, fontCharset);
        }
    }

    const QString fontFamily(styleStack.property(KOdfXmlNS::style, "font-family"));
    if (!fontFamily.isEmpty())
        fontName = fontFamily;

    if (!fontName.isEmpty()) {
        // Hmm, the remove "'" could break it's in the middle of the fontname...
        fontName = fontName.remove('\'');

        // 'Thorndale' is not known outside OpenOffice so we substitute it
        // with 'Times New Roman' that looks nearly the same.
        if (fontName == "Thorndale") {
            fontName = "Times New Roman";
        } else {
		int name_length = fontName.length() - 3;
		if (name_length > 0 && fontName.endsWith("CE")
                	&& fontName[name_length].isSpace()) {
	            fontName.chop(3);
	  }
        }
        setFontFamily(fontName);
    }

    // Specify the size of a font. The value of these attribute is either an absolute length or a percentage
    if (styleStack.hasProperty(KOdfXmlNS::fo, "font-size")) {
        qreal pointSize = styleStack.fontSize();
        if (pointSize > 0) {
            setFontPointSize(pointSize);
        }
    }
    else {
        const QString fontSizeRel(styleStack.property(KOdfXmlNS::style, "font-size-rel"));
        if (!fontSizeRel.isEmpty()) {
        // These attributes specify a relative font size change as a length such as +1pt, -3pt. It changes the font size based on the font size of the parent style.
            qreal pointSize = styleStack.fontSize() + KUnit::parseValue(fontSizeRel);
            if (pointSize > 0) {
                setFontPointSize(pointSize);
            }
        }
    }

    // Specify the weight of a font. The permitted values are normal, bold, and numeric values 100-900, in steps of 100. Unsupported numerical values are rounded off to the next supported value.
    const QString fontWeight(styleStack.property(KOdfXmlNS::fo, "font-weight"));
    if (!fontWeight.isEmpty()) {     // 3.10.24
        int boldness;
        if (fontWeight == "normal")
            boldness = 50;
        else if (fontWeight == "bold")
            boldness = 75;
        else
            // XSL/CSS has 100,200,300...900. Not the same scale as Qt!
            // See http://www.w3.org/TR/2001/REC-xsl-20011015/slice7.html#font-weight
            boldness = fontWeight.toInt() / 10;
        setFontWeight(boldness);
    }

    // Specify whether to use normal or italic font face.
    const QString fontStyle(styleStack.property(KOdfXmlNS::fo, "font-style" ));
    if (!fontStyle.isEmpty()) {     // 3.10.19
        if (fontStyle == "italic" || fontStyle == "oblique") {    // no difference in kotext
            setFontItalic(true);
        }
    }

//TODO
#if 0
    d->m_bWordByWord = styleStack.property(KOdfXmlNS::style, "text-underline-mode") == "skip-white-space";
    // TODO style:text-line-through-mode

    /*
    // OO compat code, to move to OO import filter
    d->m_bWordByWord = (styleStack.hasProperty( KOdfXmlNS::fo, "score-spaces")) // 3.10.25
                      && (styleStack.property( KOdfXmlNS::fo, "score-spaces") == "false");
    if( styleStack.hasProperty( KOdfXmlNS::style, "text-crossing-out" )) { // 3.10.6
        QString strikeOutType = styleStack.property( KOdfXmlNS::style, "text-crossing-out" );
        if( strikeOutType =="double-line")
            m_strikeOutType = S_DOUBLE;
        else if( strikeOutType =="single-line")
            m_strikeOutType = S_SIMPLE;
        else if( strikeOutType =="thick-line")
            m_strikeOutType = S_SIMPLE_BOLD;
        // not supported by KWord: "slash" and "X"
        // not supported by OO: stylelines (solid, dash, dot, dashdot, dashdotdot)
    }
    */
#endif

    const QString textUndelineMode(styleStack.property( KOdfXmlNS::style, "text-underline-mode"));
    if (!textUndelineMode.isEmpty()) {
        if (textUndelineMode == "skip-white-space") {
            setUnderlineMode(SkipWhiteSpaceLineMode);
        } else if (textUndelineMode == "continuous") {
            setUnderlineMode(ContinuousLineMode);
        }
    }

    // Specifies whether text is underlined, and if so, whether a single or qreal line will be used for underlining.
    const QString textUnderlineType(styleStack.property(KOdfXmlNS::style, "text-underline-type"));
    const QString textUnderlineStyle(styleStack.property(KOdfXmlNS::style, "text-underline-style"));
    if (!textUnderlineType.isEmpty() || !textUnderlineStyle.isEmpty()) {    // OASIS 14.4.28
        LineStyle underlineStyle;
        LineType underlineType;
        qreal underlineWidth;
        LineWeight underlineWeight;

        importOdfLine(textUnderlineType, textUnderlineStyle,
                      styleStack.property(KOdfXmlNS::style, "text-underline-width"),
                      underlineStyle, underlineType, underlineWeight, underlineWidth);
        setUnderlineStyle(underlineStyle);
        setUnderlineType(underlineType);
        setUnderlineWidth(underlineWeight, underlineWidth);
    }

    // Specifies the color that is used to underline text. The value of this attribute is either font-color or a color. If the value is font-color, the current text color is used for underlining.
    QString underLineColor = styleStack.property(KOdfXmlNS::style, "text-underline-color");   // OO 3.10.23, OASIS 14.4.31
    if (!underLineColor.isEmpty() && underLineColor != "font-color")
        setUnderlineColor(QColor(underLineColor));


    const QString textLineThroughType(styleStack.property(KOdfXmlNS::style, "text-line-through-type"));
    const QString textLineThroughStyle(styleStack.property(KOdfXmlNS::style, "text-line-through-style"));
    if (!textLineThroughType.isEmpty() || !textLineThroughStyle.isEmpty()) { // OASIS 14.4.7
        KCharacterStyle::LineStyle throughStyle;
        LineType throughType;
        qreal throughWidth;
        LineWeight throughWeight;

        importOdfLine(textLineThroughType,textLineThroughStyle,
                      styleStack.property(KOdfXmlNS::style, "text-line-through-width"),
                      throughStyle, throughType, throughWeight, throughWidth);

        setStrikeOutStyle(throughStyle);
        setStrikeOutType(throughType);
        setStrikeOutWidth(throughWeight, throughWidth);
        const QString textLineThroughText(styleStack.property(KOdfXmlNS::style, "text-line-through-text"));
        if (!textLineThroughText.isEmpty()) {
            setStrikeOutText(textLineThroughText);
        }
    }

    const QString lineThroughColor(styleStack.property(KOdfXmlNS::style, "text-line-through-color"));   // OO 3.10.23, OASIS 14.4.31
    if (!lineThroughColor.isEmpty() && lineThroughColor != "font-color") {
        setStrikeOutColor(QColor(lineThroughColor));
    }

    const QString lineThroughMode(styleStack.property(KOdfXmlNS::style, "text-line-through-mode"));
    if (lineThroughMode == "continuous") {
        setStrikeOutMode(ContinuousLineMode);
    }
    else if (lineThroughMode == "skip-white-space") {
        setStrikeOutMode(SkipWhiteSpaceLineMode);
    }

    const QString textPosition(styleStack.property(KOdfXmlNS::style, "text-position"));
    if (!textPosition.isEmpty()) {  // OO 3.10.7
        if (textPosition.startsWith("super"))
            setVerticalAlignment(QTextCharFormat::AlignSuperScript);
        else if (textPosition.startsWith("sub"))
            setVerticalAlignment(QTextCharFormat::AlignSubScript);
        else {
            QRegExp re("(-?[\\d.]+)%.*");
            if (re.exactMatch(textPosition)) {
                float value = re.capturedTexts()[1].toFloat();
                if (value > 0)
                    setVerticalAlignment(QTextCharFormat::AlignSuperScript);
                else if (value < 0)
                    setVerticalAlignment(QTextCharFormat::AlignSubScript);
            }
        }
    }

    // The fo:font-variant attribute provides the option to display text as small capitalized letters.
    const QString textVariant(styleStack.property(KOdfXmlNS::fo, "font-variant"));
    if (!textVariant.isEmpty()) {
        if (textVariant == "small-caps")
            setFontCapitalization(QFont::SmallCaps);
        else if (textVariant == "normal")
            setFontCapitalization(QFont::MixedCase);
    }
    // The fo:text-transform attribute specifies text transformations to uppercase, lowercase, and capitalization.
    else {
        const QString textTransform(styleStack.property(KOdfXmlNS::fo, "text-transform"));
        if (!textTransform.isEmpty()) {
            if (textTransform == "uppercase")
                setFontCapitalization(QFont::AllUppercase);
            else if (textTransform == "lowercase")
                setFontCapitalization(QFont::AllLowercase);
            else if (textTransform == "capitalize")
                setFontCapitalization(QFont::Capitalize);
        }
    }

    const QString foLanguage(styleStack.property(KOdfXmlNS::fo, "language"));
    if (!foLanguage.isEmpty()) {
        setLanguage(foLanguage);
    }

    const QString foCountry(styleStack.property(KOdfXmlNS::fo, "country"));
    if (!foCountry.isEmpty()) {
        setCountry(foCountry);
    }

    // The fo:background-color attribute specifies the background color of a paragraph.
    const QString bgcolor(styleStack.property(KOdfXmlNS::fo, "background-color"));
    if (!bgcolor.isEmpty()) {
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

    // The style:use-window-font-color attribute specifies whether or not the window foreground color should be as used as the foreground color for a light background color and white for a dark background color.
    if (styleStack.property(KOdfXmlNS::style, "use-window-font-color") == "true") {
        // Do like OpenOffice.org : change the foreground font if its color is too close to the background color...

        QColor back = background().color();
        QColor front = foreground().color();
        if ((abs(qGray(back.rgb()) - qGray(front.rgb())) < 10) && (background().style() != Qt::NoBrush)) {
            front.setRed(255 - front.red());
            front.setGreen(255 - front.green());
            front.setBlue(255 - front.blue());
            QBrush frontBrush = foreground();
            frontBrush.setColor(front);
            if (frontBrush.style() == Qt::NoBrush) {
                frontBrush.setStyle(Qt::SolidPattern);
            }
            setForeground(frontBrush);
        }
    }

    const QString letterKerning(styleStack.property( KOdfXmlNS::style, "letter-kerning"));
    if (!letterKerning.isEmpty()) {
        setFontKerning(letterKerning == "true");
    }

    const QString letterSpacing(styleStack.property(KOdfXmlNS::fo, "letter-spacing"));
    if (!letterSpacing.isEmpty() && letterSpacing != "100") { // 100% doesn't do anything.
        qreal space = KUnit::parseValue(letterSpacing);
        KPostscriptPaintDevice ps;
        QFontMetrics fm(font(), &ps);
        setFontLetterSpacing(100+100*space/fm.averageCharWidth());
    }

    const QString textOutline(styleStack.property(KOdfXmlNS::style, "text-outline"));
    if (!textOutline.isEmpty()) {
        if (textOutline == "true") {
            setTextOutline(QPen((foreground().style() != Qt::NoBrush)?foreground():QBrush(Qt::black) , 0));
            setForeground(Qt::transparent);
        } else {
            setTextOutline(QPen(Qt::NoPen));
        }
    }

    const QString textRotationAngle(styleStack.property(KOdfXmlNS::style, "text-rotation-angle"));
    if (!textRotationAngle.isEmpty()) {
        int angle = textRotationAngle.toInt();
        setTextRotationAngle(intToRotationAngle(angle));
    }

    const QString textRotationScale(styleStack.property(KOdfXmlNS::style, "text-rotation-scale"));
    if (!textRotationScale.isEmpty()) {
        setTextRotationScale(stringToRotationScale(textRotationScale));
    }

    const QString textScale(styleStack.property(KOdfXmlNS::style, "text-scale"));
    if (!textScale.isEmpty()) {
        const int scale = (textScale.endsWith('%') ? textScale.left(textScale.length()-1) : textScale).toInt();
        setTextScale(scale);
    }

//TODO
#if 0
    if (styleStack.hasProperty(KOdfXmlNS::fo, "text-shadow")) {    // 3.10.21
        parseShadowFromCss(styleStack.property(KOdfXmlNS::fo, "text-shadow"));
    }

    d->m_bHyphenation = true;
    if (styleStack.hasProperty(KOdfXmlNS::fo, "hyphenate"))     // it's a character property in OASIS (but not in OO-1.1)
        d->m_bHyphenation = styleStack.property(KOdfXmlNS::fo, "hyphenate") == "true";

    /*
      Missing properties:
      style:font-style-name, 3.10.11 - can be ignored, says DV, the other ways to specify a font are more precise
      style:text-relief, 3.10.20 - not implemented in kotext
      style:text-blinking, 3.10.27 - not implemented in kotext IIRC
      style:text-combine, 3.10.29/30 - not implemented, see http://www.w3.org/TR/WD-i18n-format/
      style:text-emphasis, 3.10.31 - not implemented in kotext
      style:text-scale, 3.10.33 - not implemented in kotext
      style:text-rotation-angle, 3.10.34 - not implemented in kotext (kpr rotates whole objects)
      style:text-rotation-scale, 3.10.35 - not implemented in kotext (kpr rotates whole objects)
      style:punctuation-wrap, 3.10.36 - not implemented in kotext
    */

    d->m_underLineWidth = 1.0;

    generateKey();
    addRef();
#endif
}

bool KCharacterStyle::operator==(const KCharacterStyle &other) const
{
    return other.d->stylesPrivate == d->stylesPrivate;
}

void KCharacterStyle::removeDuplicates(const KCharacterStyle &other)
{
    this->d->stylesPrivate.removeDuplicates(other.d->stylesPrivate);
}

void KCharacterStyle::removeDuplicates(const QTextCharFormat &otherFormat)
{
    KCharacterStyle other(otherFormat);
    removeDuplicates(other);
}

bool KCharacterStyle::isEmpty() const
{
    return d->stylesPrivate.isEmpty();
}

void KCharacterStyle::saveOdf(KOdfGenericStyle &style)
{
    if (!d->name.isEmpty() && !style.isDefaultStyle()) {
        style.addAttribute("style:display-name", d->name);
    }
    QList<int> keys = d->stylesPrivate.keys();
    foreach(int key, keys) {
        if (key == QTextFormat::FontWeight) {
            bool ok = false;
            int boldness = d->stylesPrivate.value(key).toInt(&ok);
            if (ok) {
                if (boldness == QFont::Normal) {
                    style.addProperty("fo:font-weight", "normal", KOdfGenericStyle::TextType);
                } else if (boldness == QFont::Bold) {
                    style.addProperty("fo:font-weight", "bold", KOdfGenericStyle::TextType);
                } else {
                    // Remember : Qt and CSS/XSL doesn't have the same scale. Its 100-900 instead of Qts 0-100
                    style.addProperty("fo:font-weight", qBound(10, boldness, 90) * 10, KOdfGenericStyle::TextType);
                }
            }
        } else if (key == QTextFormat::FontItalic) {
            if (d->stylesPrivate.value(key).toBool()) {
                style.addProperty("fo:font-style", "italic", KOdfGenericStyle::TextType);
            } else {
                style.addProperty("fo:font-style", "", KOdfGenericStyle::TextType);
            }
        } else if (key == QTextFormat::FontFamily) {
            QString fontFamily = d->stylesPrivate.value(key).toString();
            style.addProperty("fo:font-family", fontFamily, KOdfGenericStyle::TextType);
        } else if (key == QTextFormat::FontFixedPitch) {
            bool fixedPitch = d->stylesPrivate.value(key).toBool();
            style.addProperty("style:font-pitch", fixedPitch ? "fixed" : "variable", KOdfGenericStyle::TextType);
        } else if (key == QTextFormat::FontStyleHint) {
            bool ok = false;
            int styleHint = d->stylesPrivate.value(key).toInt(&ok);
            if (ok)
                style.addProperty("style:font-family-generic", exportOdfFontStyleHint((QFont::StyleHint) styleHint), KOdfGenericStyle::TextType);
        } else if (key == QTextFormat::FontKerning) {
            style.addProperty("style:letter-kerning", fontKerning() ? "true" : "false", KOdfGenericStyle::TextType);
        } else if (key == QTextFormat::FontCapitalization) {
            switch (fontCapitalization()) {
            case QFont::SmallCaps:
                style.addProperty("fo:font-variant", "small-caps", KOdfGenericStyle::TextType);
                break;
            case QFont::MixedCase:
                style.addProperty("fo:font-variant", "normal", KOdfGenericStyle::TextType);
                break;
            case QFont::AllUppercase:
                style.addProperty("fo:text-transform", "uppercase", KOdfGenericStyle::TextType);
                break;
            case QFont::AllLowercase:
                style.addProperty("fo:text-transform", "lowercase", KOdfGenericStyle::TextType);
                break;
            case QFont::Capitalize:
                style.addProperty("fo:text-transform", "capitalize", KOdfGenericStyle::TextType);
                break;
            }
        } else if (key == UnderlineStyle) {
            bool ok = false;
            int styleId = d->stylesPrivate.value(key).toInt(&ok);
            if (ok)
                style.addProperty("style:text-underline-style", exportOdfLineStyle((KCharacterStyle::LineStyle) styleId), KOdfGenericStyle::TextType);
        } else if (key == UnderlineType) {
            bool ok = false;
            int type = d->stylesPrivate.value(key).toInt(&ok);
            if (ok)
                style.addProperty("style:text-underline-type", exportOdfLineType((KCharacterStyle::LineType) type), KOdfGenericStyle::TextType);
        } else if (key == QTextFormat::TextUnderlineColor) {
            QColor color = d->stylesPrivate.value(key).value<QColor>();
            if (color.isValid())
                style.addProperty("style:text-underline-color", color.name(), KOdfGenericStyle::TextType);
        } else if (key == UnderlineMode) {
            bool ok = false;
            int mode = d->stylesPrivate.value(key).toInt(&ok);
            if (ok)
                style.addProperty("style:text-underline-mode", exportOdfLineMode((KCharacterStyle::LineMode) mode), KOdfGenericStyle::TextType);
        } else if (key == UnderlineWidth) {
            KCharacterStyle::LineWeight weight;
            qreal width;
            underlineWidth(weight, width);
            style.addProperty("style:text-underline-width", exportOdfLineWidth(weight, width), KOdfGenericStyle::TextType);
        } else if (key == StrikeOutStyle) {
            bool ok = false;
            int styleId = d->stylesPrivate.value(key).toInt(&ok);
            if (ok)
                style.addProperty("style:text-line-through-style", exportOdfLineStyle((KCharacterStyle::LineStyle) styleId), KOdfGenericStyle::TextType);
        } else if (key == StrikeOutType) {
            bool ok = false;
            int type = d->stylesPrivate.value(key).toInt(&ok);
            if (ok)
                style.addProperty("style:text-line-through-type", exportOdfLineType((KCharacterStyle::LineType) type), KOdfGenericStyle::TextType);
        } else if (key == StrikeOutText) {
            style.addProperty("style:text-line-through-text", d->stylesPrivate.value(key).toString(), KOdfGenericStyle::TextType);
        } else if (key == StrikeOutColor) {
            QColor color = d->stylesPrivate.value(key).value<QColor>();
            if (color.isValid())
                style.addProperty("style:text-line-through-color", color.name(), KOdfGenericStyle::TextType);
        } else if (key == StrikeOutMode) {
            bool ok = false;
            int mode = d->stylesPrivate.value(key).toInt(&ok);
            if (ok)
                style.addProperty("style:text-line-through-mode", exportOdfLineMode((KCharacterStyle::LineMode) mode), KOdfGenericStyle::TextType);
        } else if (key == StrikeOutWidth) {
            KCharacterStyle::LineWeight weight;
            qreal width;
            strikeOutWidth(weight, width);
            style.addProperty("style:text-line-through-width", exportOdfLineWidth(weight, width), KOdfGenericStyle::TextType);
        } else if (key == QTextFormat::BackgroundBrush) {
            QBrush brush = d->stylesPrivate.value(key).value<QBrush>();
            if (brush.style() == Qt::NoBrush)
                style.addProperty("fo:background-color", "transparent", KOdfGenericStyle::TextType);
            else
                style.addProperty("fo:background-color", brush.color().name(), KOdfGenericStyle::TextType);
        } else if (key == QTextFormat::ForegroundBrush) {
            QBrush brush = d->stylesPrivate.value(key).value<QBrush>();
            if (brush.style() == Qt::NoBrush)
                style.addProperty("fo:color", "transparent", KOdfGenericStyle::TextType);
            else
                style.addProperty("fo:color", brush.color().name(), KOdfGenericStyle::TextType);
        } else if (key == QTextFormat::TextVerticalAlignment) {
            if (verticalAlignment() == QTextCharFormat::AlignSuperScript)
                style.addProperty("style:text-position", "super", KOdfGenericStyle::TextType);
            else if (verticalAlignment() == QTextCharFormat::AlignSubScript)
                style.addProperty("style:text-position", "sub", KOdfGenericStyle::TextType);
        } else if (key == QTextFormat::FontPointSize) {
            style.addPropertyPt("fo:font-size", fontPointSize(), KOdfGenericStyle::TextType);
        } else if (key == KCharacterStyle::Country) {
            style.addProperty("fo:country", d->stylesPrivate.value(KCharacterStyle::Country).toString(), KOdfGenericStyle::TextType);
        } else if (key == KCharacterStyle::Language) {
            style.addProperty("fo:language", d->stylesPrivate.value(KCharacterStyle::Language).toString(), KOdfGenericStyle::TextType);
        } else if (key == QTextCharFormat::FontLetterSpacing) {
            // in Qt letter-spacing is in percent, in ODF its a static value.
            // We are just approximating the conversion here...
            KPostscriptPaintDevice ps;
            QFontMetrics fm(font(), &ps);
            const qreal spacing = ((fontLetterSpacing() - 100) * fm.averageCharWidth()) / 100.;
            style.addPropertyPt("fo:letter-spacing", spacing, KOdfGenericStyle::TextType);
        } else if (key == QTextFormat::TextOutline) {
            QPen outline = textOutline();
            style.addProperty("style:text-outline", outline.style() == Qt::NoPen ? "false" : "true", KOdfGenericStyle::TextType);
        } else if (key == KCharacterStyle::FontCharset) {
            style.addProperty("style:font-charset", d->stylesPrivate.value(KCharacterStyle::FontCharset).toString(), KOdfGenericStyle::TextType);
        } else if (key == KCharacterStyle::TextRotationAngle) {
            RotationAngle angle = textRotationAngle();
            style.addProperty("style:text-rotation-angle", rotationAngleToInt(angle), KOdfGenericStyle::TextType);
        } else if (key == KCharacterStyle::TextRotationScale) {
            RotationScale scale = textRotationScale();
            style.addProperty("style:text-rotation-scale", rotationScaleToString(scale), KOdfGenericStyle::TextType);
        } else if (key == KCharacterStyle::TextScale) {
            int scale = textScale();
            style.addProperty("style:text-scale", QString::number(scale) + '%', KOdfGenericStyle::TextType);
        }
    }
    //TODO: font name and family
}

QVariant KCharacterStyle::value(int key) const
{
    return d->stylesPrivate.value(key);
}

#include <KCharacterStyle.moc>
