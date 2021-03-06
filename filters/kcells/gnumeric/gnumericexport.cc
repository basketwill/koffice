/* This file is part of the KDE project
   Copyright (C) 2000 David Faure <faure@kde.org>
   Copyright (C) 2005 Laurent Montel <montel@kde.org>

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

/* Gnumeric export filter by Phillip Ezolt (phillipezolt@hotmail.com)
   2004 - Some updates by Tim Beaulen (tbscope@gmail.com) */

#include <gnumericexport.h>
#include <kdebug.h>
#include <kfilterdev.h>
#include <kmessagebox.h>
#include <kpluginfactory.h>
#include <KoFilterChain.h>
#include <KoZoomHandler.h>
#include <QApplication>
#include <QList>
#include <QFile>
#include <QTextStream>
#include <QPen>
#include <QByteArray>

// KOffice
#include <KoDocumentInfo.h>

// KCells
#include <kcells/KCApplicationSettings.h>
#include <kcells/part/KCCanvas.h>
#include <kcells/KCCellStorage.h>
#include <kcells/KCCurrency.h>
#include <kcells/part/KCDoc.h>
#include <kcells/KCHeaderFooter.h>
#include <kcells/KCMap.h>
#include <kcells/KCNamedAreaManager.h>
#include <kcells/KCPrintSettings.h>
#include <kcells/KCSheet.h>
#include <kcells/RowColumnFormat.h>
#include <kcells/KCValidity.h>
#include <kcells/KCValueConverter.h>
#include <kcells/part/KCView.h>

K_PLUGIN_FACTORY(GNUMERICExportFactory, registerPlugin<GNUMERICExport>();)
K_EXPORT_PLUGIN(GNUMERICExportFactory("kofficefilters"))

GNUMERICExport::GNUMERICExport(QObject* parent, const QVariantList&)
        : KoFilter(parent)
{
    isLink = false;
    isLinkBold = false;
    isLinkItalic = false;
}

/**
 * This function will check if a cell has any type of border.
 */
bool GNUMERICExport::hasBorder(const KCCell& cell, int currentcolumn, int currentrow)
{
    Q_UNUSED(currentcolumn);
    Q_UNUSED(currentrow);
    const KCStyle style = cell.style();
    if (((style.leftBorderPen().width() != 0) &&
            (style.leftBorderPen().style() != Qt::NoPen)) ||
            ((style.rightBorderPen().width() != 0) &&
             (style.rightBorderPen().style() != Qt::NoPen)) ||
            ((style.topBorderPen().width() != 0) &&
             (style.topBorderPen().style() != Qt::NoPen)) ||
            ((style.bottomBorderPen().width() != 0) &&
             (style.bottomBorderPen().style() != Qt::NoPen)) ||
            ((style.fallDiagonalPen().width() != 0) &&
             (style.fallDiagonalPen().style() != Qt::NoPen)) ||
            ((style.goUpDiagonalPen().width() != 0) &&
             (style.goUpDiagonalPen().style() != Qt::NoPen)))
        return true;
    else
        return false;
}

const QString GNUMERICExport::ColorToString(int red, int green, int blue)
{
    return QString::number(red, 16) + ':' + QString::number(green, 16) + ':' + QString::number(blue, 16);
}

QDomElement GNUMERICExport::GetBorderStyle(QDomDocument gnumeric_doc, const KCCell& cell, int currentcolumn, int currentrow)
{
    Q_UNUSED(currentcolumn);
    Q_UNUSED(currentrow);

    QDomElement border_style;
    QDomElement border;

    int red, green, blue;
    QColor color;

    border_style = gnumeric_doc.createElement("gmr:StyleBorder");
    const KCStyle style = cell.style();

    if ((style.leftBorderPen().width() != 0) &&
            (style.leftBorderPen().style() != Qt::NoPen)) {
        border = gnumeric_doc.createElement("gmr:Left");
        border.setAttribute("Style", "1");

        color =  style.leftBorderPen().color();
        red = color.red() << 8;
        green = color.green() << 8;
        blue = color.blue() << 8;

        border.setAttribute("Color", QString::number(red, 16) + ':' + QString::number(green, 16) + ':' + QString::number(blue, 16));
    } else {
        border = gnumeric_doc.createElement("gmr:Left");
        border.setAttribute("Style", "0");
    }

    border_style.appendChild(border);

    if ((style.rightBorderPen().width() != 0) &&
            (style.rightBorderPen().style() != Qt::NoPen)) {
        border = gnumeric_doc.createElement("gmr:Right");
        border.setAttribute("Style", "1");

        color =  style.rightBorderPen().color();
        red = color.red() << 8;
        green = color.green() << 8;
        blue = color.blue() << 8;

        border.setAttribute("Color", QString::number(red, 16) + ':' + QString::number(green, 16) + ':' + QString::number(blue, 16));
    } else {
        border = gnumeric_doc.createElement("gmr:Right");
        border.setAttribute("Style", "0");
    }

    border_style.appendChild(border);

    if ((style.topBorderPen().width() != 0) &&
            (style.topBorderPen().style() != Qt::NoPen)) {
        border = gnumeric_doc.createElement("gmr:Top");
        border.setAttribute("Style", "1");

        color =  style.topBorderPen().color();
        red = color.red() << 8;
        green = color.green() << 8;
        blue = color.blue() << 8;

        border.setAttribute("Color", QString::number(red, 16) + ':' + QString::number(green, 16) + ':' + QString::number(blue, 16));
    } else {
        border = gnumeric_doc.createElement("gmr:Top");
        border.setAttribute("Style", "0");
    }

    border_style.appendChild(border);

    if ((style.bottomBorderPen().width() != 0) &&
            (style.bottomBorderPen().style() != Qt::NoPen)) {
        border = gnumeric_doc.createElement("gmr:Bottom");
        border.setAttribute("Style", "1");

        color =  style.bottomBorderPen().color();
        red = color.red() << 8;
        green = color.green() << 8;
        blue = color.blue() << 8;

        border.setAttribute("Color", QString::number(red, 16) + ':' + QString::number(green, 16) + ':' + QString::number(blue, 16));
    } else {
        border = gnumeric_doc.createElement("gmr:Bottom");
        border.setAttribute("Style", "0");
    }

    border_style.appendChild(border);

    if ((style.fallDiagonalPen().width() != 0) &&
            (style.fallDiagonalPen().style() != Qt::NoPen)) {
        border = gnumeric_doc.createElement("gmr:Diagonal");
        border.setAttribute("Style", "1");

        color =  style.fallDiagonalPen().color();
        red = color.red() << 8;
        green = color.green() << 8;
        blue = color.blue() << 8;

        border.setAttribute("Color", QString::number(red, 16) + ':' + QString::number(green, 16) + ':' + QString::number(blue, 16));
    } else {
        border = gnumeric_doc.createElement("gmr:Diagonal");
        border.setAttribute("Style", "0");
    }

    border_style.appendChild(border);

    if ((style.goUpDiagonalPen().width() != 0) &&
            (style.goUpDiagonalPen().style() != Qt::NoPen)) {
        border = gnumeric_doc.createElement("gmr:Rev-Diagonal");
        border.setAttribute("Style", "1");

        color =  style.goUpDiagonalPen().color();
        red = color.red() << 8;
        green = color.green() << 8;
        blue = color.blue() << 8;

        border.setAttribute("Color", QString::number(red, 16) + ':' + QString::number(green, 16) + ':' + QString::number(blue, 16));
    } else {
        border = gnumeric_doc.createElement("gmr:Rev-Diagonal");
        border.setAttribute("Style", "0");
    }

    border_style.appendChild(border);

    return border_style;
}

QDomElement GNUMERICExport::GetValidity(QDomDocument gnumeric_doc, const KCCell& cell)
{
    KCValueConverter *const converter = cell.sheet()->map()->converter();

    //<gmr:Validation Style="1" Type="1" Operator="7" AllowBlank="true" UseDropdown="false" Title="ghhg" Message="ghghhhjfhfghjfghj&#10;fg&#10;hjgf&#10;hj">
    //        <gmr:Expression0>45</gmr:Expression0>
    //      </gmr:Validation>
    KCValidity kcells_validity = cell.validity();
    QDomElement val = gnumeric_doc.createElement("gmr:Validation");
    val.setAttribute("Title", kcells_validity.title());
    val.setAttribute("Message", kcells_validity.message());
    val.setAttribute("AllowBlank", kcells_validity.allowEmptyCell() ? "true" : "false");
    if (!kcells_validity.displayMessage()) {
        val.setAttribute("Style", "0");
    } else {
        switch (kcells_validity.action()) {
        case KCValidity::Stop:
            val.setAttribute("Style", "1");
            break;
        case KCValidity::Warning:
            val.setAttribute("Style", "2");
            break;
        case KCValidity::Information:
            val.setAttribute("Style", "3");
            break;
        }
    }

    switch (kcells_validity.condition()) {
    case KCConditional::None:
        //Nothing
        break;
    case KCConditional::Equal:
        val.setAttribute("Operator", "2");
        break;
    case KCConditional::Superior:
        val.setAttribute("Operator", "4");
        break;
    case KCConditional::Inferior:
        val.setAttribute("Operator", "5");
        break;
    case KCConditional::SuperiorEqual:
        val.setAttribute("Operator", "6");
        break;
    case KCConditional::InferiorEqual:
        val.setAttribute("Operator", "7");
        break;
    case KCConditional::Between:
        val.setAttribute("Operator", "0");
        break;
    case KCConditional::Different:
        val.setAttribute("Operator", "3");
        break;
    case KCConditional::DifferentTo:
        val.setAttribute("Operator", "1");
        break;
    }
    switch (kcells_validity.restriction()) {
    case KCValidity::None:
        val.setAttribute("Type", "0");
        break;
    case KCValidity::KCNumber: {
        val.setAttribute("Type", "2");
        switch (kcells_validity.condition()) {
        case KCConditional::None:
            //Nothing
            break;
        case KCConditional::Equal:
        case KCConditional::Superior:
        case KCConditional::Inferior:
        case KCConditional::SuperiorEqual:
        case KCConditional::InferiorEqual:
        case KCConditional::Different: {
            QDomElement tmp = gnumeric_doc.createElement("gmr:Expression0");
            tmp.appendChild(gnumeric_doc.createTextNode(converter->asString(kcells_validity.minimumValue()).asString()));
            val.appendChild(tmp);
        }
        break;
        case KCConditional::Between:
        case KCConditional::DifferentTo: {
            QDomElement tmp = gnumeric_doc.createElement("gmr:Expression0");
            tmp.appendChild(gnumeric_doc.createTextNode(converter->asString(kcells_validity.minimumValue()).asString()));
            val.appendChild(tmp);
            tmp = gnumeric_doc.createElement("gmr:Expression1");
            tmp.appendChild(gnumeric_doc.createTextNode(converter->asString(kcells_validity.maximumValue()).asString()));
            val.appendChild(tmp);
        }
        break;
        }

        break;
    }
    case KCValidity::Text:
        //Not supported into gnumeric
        //val.setAttribute("Type", "1" );
        break;
    case KCValidity::Time:
        val.setAttribute("Type", "5");
        switch (kcells_validity.condition()) {
        case KCConditional::None:
            //Nothing
            break;
        case KCConditional::Equal:
        case KCConditional::Superior:
        case KCConditional::Inferior:
        case KCConditional::SuperiorEqual:
        case KCConditional::InferiorEqual:
        case KCConditional::Different: {
            QDomElement tmp = gnumeric_doc.createElement("gmr:Expression0");
            tmp.appendChild(gnumeric_doc.createTextNode(converter->asString(kcells_validity.minimumValue()).asString()));
            val.appendChild(tmp);
        }
        break;
        case KCConditional::Between:
        case KCConditional::DifferentTo: {
            QDomElement tmp = gnumeric_doc.createElement("gmr:Expression0");
            tmp.appendChild(gnumeric_doc.createTextNode(converter->asString(kcells_validity.minimumValue()).asString()));
            val.appendChild(tmp);
            tmp = gnumeric_doc.createElement("gmr:Expression1");
            tmp.appendChild(gnumeric_doc.createTextNode(converter->asString(kcells_validity.maximumValue()).asString()));
            val.appendChild(tmp);
        }
        break;
        }

        break;
    case KCValidity::Date:
        val.setAttribute("Type", "4");
        switch (kcells_validity.condition()) {
        case KCConditional::None:
            //Nothing
            break;
        case KCConditional::Equal:
        case KCConditional::Superior:
        case KCConditional::Inferior:
        case KCConditional::SuperiorEqual:
        case KCConditional::InferiorEqual:
        case KCConditional::Different: {
            QDomElement tmp = gnumeric_doc.createElement("gmr:Expression0");
            tmp.appendChild(gnumeric_doc.createTextNode(converter->asString(kcells_validity.minimumValue()).asString()));
            val.appendChild(tmp);
        }
        break;
        case KCConditional::Between:
        case KCConditional::DifferentTo: {
            QDomElement tmp = gnumeric_doc.createElement("gmr:Expression0");
            tmp.appendChild(gnumeric_doc.createTextNode(converter->asString(kcells_validity.minimumValue()).asString()));
            val.appendChild(tmp);
            tmp = gnumeric_doc.createElement("gmr:Expression1");
            tmp.appendChild(gnumeric_doc.createTextNode(converter->asString(kcells_validity.maximumValue()).asString()));
            val.appendChild(tmp);
        }
        break;
        }

        break;
    case KCValidity::Integer:
        val.setAttribute("Type", "1");
        switch (kcells_validity.condition()) {
        case KCConditional::None:
            //Nothing
            break;
        case KCConditional::Equal:
        case KCConditional::Superior:
        case KCConditional::Inferior:
        case KCConditional::SuperiorEqual:
        case KCConditional::InferiorEqual:
        case KCConditional::Different: {
            QDomElement tmp = gnumeric_doc.createElement("gmr:Expression0");
            tmp.appendChild(gnumeric_doc.createTextNode(converter->asString(kcells_validity.minimumValue()).asString()));
            val.appendChild(tmp);
        }
        break;
        case KCConditional::Between:
        case KCConditional::DifferentTo: {
            QDomElement tmp = gnumeric_doc.createElement("gmr:Expression0");
            tmp.appendChild(gnumeric_doc.createTextNode(converter->asString(kcells_validity.minimumValue()).asString()));
            val.appendChild(tmp);
            tmp = gnumeric_doc.createElement("gmr:Expression1");
            tmp.appendChild(gnumeric_doc.createTextNode(converter->asString(kcells_validity.maximumValue()).asString()));
            val.appendChild(tmp);
        }
        break;
        }
        break;
    case KCValidity::TextLength:
        val.setAttribute("Type", "6");
        switch (kcells_validity.condition()) {
        case KCConditional::None:
            //Nothing
            break;
        case KCConditional::Equal:
        case KCConditional::Superior:
        case KCConditional::Inferior:
        case KCConditional::SuperiorEqual:
        case KCConditional::InferiorEqual:
        case KCConditional::Different: {
            QDomElement tmp = gnumeric_doc.createElement("gmr:Expression0");
            tmp.appendChild(gnumeric_doc.createTextNode(converter->asString(kcells_validity.minimumValue()).asString()));
            val.appendChild(tmp);
        }
        break;
        case KCConditional::Between:
        case KCConditional::DifferentTo: {
            QDomElement tmp = gnumeric_doc.createElement("gmr:Expression0");
            tmp.appendChild(gnumeric_doc.createTextNode(converter->asString(kcells_validity.minimumValue()).asString()));
            val.appendChild(tmp);
            tmp = gnumeric_doc.createElement("gmr:Expression1");
            tmp.appendChild(gnumeric_doc.createTextNode(converter->asString(kcells_validity.maximumValue()).asString()));
            val.appendChild(tmp);
        }
        break;
        }
        break;
    case KCValidity::List:
        val.setAttribute("Type", "3");
        switch (kcells_validity.condition()) {
        case KCConditional::None:
            //Nothing
            break;
        case KCConditional::Equal:
        case KCConditional::Superior:
        case KCConditional::Inferior:
        case KCConditional::SuperiorEqual:
        case KCConditional::InferiorEqual:
        case KCConditional::Different:
            break;
        case KCConditional::Between:
        case KCConditional::DifferentTo:
            break;
        }
        break;
    }

    return val;
}

QDomElement GNUMERICExport::GetFontStyle(QDomDocument gnumeric_doc, const KCCell& cell, int currentcolumn, int currentrow)
{
    QDomElement font_style;
    const KCStyle style = cell.style();
    kDebug(30521) << " currentcolumn :" << currentcolumn << " currentrow :" << currentrow;
    font_style = gnumeric_doc.createElement("gmr:Font");
    font_style.appendChild(gnumeric_doc.createTextNode(style.fontFamily()));

    if (style.italic() || (isLink && isLinkItalic)) {
        font_style.setAttribute("Italic", "1");
    }
    if (style.bold() || (isLink && isLinkBold)) {
        font_style.setAttribute("Bold", "1");
    }
    if (style.underline()) {
        font_style.setAttribute("Underline", "1");
    }
    if (style.strikeOut()) {
        font_style.setAttribute("StrikeThrough", "1");
    }
    if (style.fontSize()) {
        font_style.setAttribute("Unit", QString::number(style.fontSize()));
    }

    return font_style;
}

QDomElement GNUMERICExport::GetLinkStyle(QDomDocument gnumeric_doc)
{
    QDomElement link_style;

    link_style = gnumeric_doc.createElement("gmr:HyperLink");

    QString path;

    path = linkUrl;

    if (path.section(':', 0, 0).toLower() == "http")
        link_style.setAttribute("type", "GnmHLinkURL");
    else if (path.section(':', 0, 0).toLower() == "mailto")
        link_style.setAttribute("type", "GnmHLinkEMail");
    else if (path.section(':', 0, 0).toLower() == "file")
        link_style.setAttribute("type", "GnmHLinkExternal");
    else if (path.left(5).toLower() == "sheet")
        link_style.setAttribute("type", "GnmHLinkCurWB");
    else
        link_style.setAttribute("type", "");

    link_style.setAttribute("target", path);

    // KCells doesn't support link tips.
    link_style.setAttribute("tip", "");

    return link_style;
}

QDomElement GNUMERICExport::GetCellStyle(QDomDocument gnumeric_doc, const KCCell& cell, int currentcolumn, int currentrow)
{
    QDomElement cell_style;

    cell_style = gnumeric_doc.createElement("gmr:Style");

    int red, green, blue;

    const KCStyle style = cell.style();
    QColor bgColor =  style.backgroundColor();
    if (bgColor.isValid()) {
        red = bgColor.red() << 8;
        green = bgColor.green() << 8;
        blue = bgColor.blue() << 8;
    } else {
        // Let's use white default for background
        red = 0xffff;
        green = 0xffff;
        blue = 0xffff;
    }

    switch (style.backgroundBrush().style()) {
    case Qt::NoBrush:
        cell_style.setAttribute("Shade", "0");
        break;
    case Qt::SolidPattern:
        // 100%
        cell_style.setAttribute("Shade", "1");
        break;
    case Qt::Dense1Pattern:
        // 87.5%
        cell_style.setAttribute("Shade", "25");
        break;
    case Qt::Dense2Pattern:
        // 75%
        cell_style.setAttribute("Shade", "2");
        break;
    case Qt::Dense3Pattern:
        // 62.5%
        // Not supported by GNumeric
        // Fall back to 50%
        cell_style.setAttribute("Shade", "3");
        break;
    case Qt::Dense4Pattern:
        // 50%
        cell_style.setAttribute("Shade", "3");
        break;
    case Qt::Dense5Pattern:
        // 25%
        cell_style.setAttribute("Shade", "4");
        break;
    case Qt::Dense6Pattern:
        // 12.5%
        cell_style.setAttribute("Shade", "5");
        break;
    case Qt::Dense7Pattern:
        // 6.25%
        cell_style.setAttribute("Shade", "6");
        break;
    case Qt::HorPattern:
        cell_style.setAttribute("Shade", "13");
        break;
    case Qt::VerPattern:
        cell_style.setAttribute("Shade", "14");
        break;
    case Qt::CrossPattern:
        cell_style.setAttribute("Shade", "17");
        break;
    case Qt::BDiagPattern:
        cell_style.setAttribute("Shade", "16");
        break;
    case Qt::FDiagPattern:
        cell_style.setAttribute("Shade", "15");
        break;
    case Qt::DiagCrossPattern:
        cell_style.setAttribute("Shade", "18");
        break;
    case Qt::TexturePattern:
    default:
        // Not supported by Gnumeric
        cell_style.setAttribute("Shade", "0");
        break;
    }

    cell_style.setAttribute("Back", QString::number(red, 16) + ':' + QString::number(green, 16) + ':' + QString::number(blue, 16));


    QColor textColor =  style.fontColor();
    red = textColor.red() << 8;
    green = textColor.green() << 8;
    blue = textColor.blue() << 8;

    cell_style.setAttribute("Fore", QString::number(red, 16) + ':' + QString::number(green, 16) + ':' + QString::number(blue, 16));

    if (style.halign() ==  KCStyle::HAlignUndefined) {
        cell_style.setAttribute("HAlign", "1");
    } else if (style.halign() ==  KCStyle::Left) {
        cell_style.setAttribute("HAlign", "2");
    } else if (style.halign() ==  KCStyle::Right) {
        cell_style.setAttribute("HAlign", "4");
    } else if (style.halign() ==  KCStyle::Center) {
        cell_style.setAttribute("HAlign", "8");
    }

    if (style.valign() ==  KCStyle::Top) {
        cell_style.setAttribute("VAlign", "1");
    } else if (style.valign() ==  KCStyle::Bottom) {
        cell_style.setAttribute("VAlign", "2");
    } else if (style.valign() ==  KCStyle::Middle) {
        cell_style.setAttribute("VAlign", "4");
    }

    if (style.wrapText())
        cell_style.setAttribute("WrapText", "1");
    else
        cell_style.setAttribute("WrapText", "0");

    // ShrinkToFit not supported by KCells (?)
    cell_style.setAttribute("ShrinkToFit", "0");

    // I'm not sure about the rotation values.
    // I never got it to work in GNumeric.
    cell_style.setAttribute("Rotation", QString::number(-1*style.angle()));

    // The indentation in GNumeric is an integer value. In KCells, it's a double.
    // Save the double anyway, makes it even better when importing the document back in KCells.
    // TODO verify if it's correct, in import we "* 10.0"
    cell_style.setAttribute("Indent", QString::number(style.indentation()));

    cell_style.setAttribute("Locked", !style.notProtected());

    // A KCells cell can have two options to hide: only formula hidden, or everything hidden.
    // I only consider a cell with everything hidden as hidden.
    // Gnumeric hides everything or nothing.
    cell_style.setAttribute("Hidden", style.hideAll());

    QColor patColor =  style.backgroundBrush().color();
    red = patColor.red() << 8;
    green = patColor.green() << 8;
    blue = patColor.blue() << 8;

    cell_style.setAttribute("PatternColor", QString::number(red, 16) + ':' + QString::number(green, 16) + ':' + QString::number(blue, 16));

    if (isLink)
        cell_style.appendChild(GetLinkStyle(gnumeric_doc));

    cell_style.appendChild(GetFontStyle(gnumeric_doc, cell, currentcolumn, currentrow));

    if (!cell.validity().isEmpty()) {
        cell_style.appendChild(GetValidity(gnumeric_doc, cell));
    }

    QString stringFormat;

    KCCurrency currency;

    switch (style.formatType()) {
    case KCFormat::Generic:
        stringFormat = "General";
        break;
    case KCFormat::KCNumber:
        stringFormat = "0.00";
        break;
    case KCFormat::Text:
        stringFormat = "General";
        break;
    case KCFormat::Money:

        if (!style.hasAttribute(KCStyle::CurrencyFormat)) {
            stringFormat = "0.00";
            break;
        }
        currency = style.currency();

        if (currency.code().isEmpty())
            stringFormat = "0.00";
        else if (currency.code() == "$")
            stringFormat = "$0.00";
        else if (currency.code() == QString::fromUtf8("€"))
            stringFormat = "[$€-2]0.00";
        else if (currency.code() == QString::fromUtf8("£"))
            stringFormat = "£0.00";
        else if (currency.code() == QString::fromUtf8("¥"))
            stringFormat = "¥0.00";
        else
            stringFormat = "[$" + currency.code() + "]0.00";

        break;
    case KCFormat::Percentage:
        stringFormat = "0.00%";
        break;
    case KCFormat::Scientific:
        stringFormat = "0.00E+00";
        break;
    case KCFormat::ShortDate:
        stringFormat = cell.locale()->dateFormatShort();
        break;
    case KCFormat::TextDate:
        stringFormat = cell.locale()->dateFormat();
        break;
    case KCFormat::Date1:
        stringFormat = "dd-mmm-yy";
        break;
    case KCFormat::Date2:
        stringFormat = "dd-mmm-yyyy";
        break;
    case KCFormat::Date3:
        stringFormat = "dd-mmm";
        break;
    case KCFormat::Date4:
        stringFormat = "dd-mm";
        break;
    case KCFormat::Date5:
        stringFormat = "dd/mm/yy";
        break;
    case KCFormat::Date6:
        stringFormat = "dd/mm/yyyy";
        break;
    case KCFormat::Date7:
        stringFormat = "mmm-yy";
        break;
    case KCFormat::Date8:
        stringFormat = "mmmm-yy";
        break;
    case KCFormat::Date9:
        stringFormat = "mmmm-yyyy";
        break;
    case KCFormat::Date10:
        stringFormat = "m-yy";
        break;
    case KCFormat::Date11:
        stringFormat = "dd/mmm";
        break;
    case KCFormat::Date12:
        stringFormat = "dd/mm";
        break;
    case KCFormat::Date13:
        stringFormat = "dd/mmm/yyyy";
        break;
    case KCFormat::Date14:
        stringFormat = "yyyy/mmm/dd";
        break;
    case KCFormat::Date15:
        stringFormat = "yyyy-mmm-dd";
        break;
    case KCFormat::Date16:
        stringFormat = "yyyy-mm-dd";
        break;
    case KCFormat::Date17:
        stringFormat = "d mmmm yyyy";
        break;
    case KCFormat::Date18:
        stringFormat = "mm/dd/yyyy";
        break;
    case KCFormat::Date19:
        stringFormat = "mm/dd/yy";
        break;
    case KCFormat::Date20:
        stringFormat = "mmm/dd/yy";
        break;
    case KCFormat::Date21:
        stringFormat = "mmm/dd/yyyy";
        break;
    case KCFormat::Date22:
        stringFormat = "mmm-yyyy";
        break;
    case KCFormat::Date23:
        stringFormat = "yyyy";
        break;
    case KCFormat::Date24:
        stringFormat = "yy";
        break;
    case KCFormat::Date25:
        stringFormat = "yyyy/mm/dd";
        break;
    case KCFormat::Date26:
        stringFormat = "yyyy/mmm/dd";
        break;
    case KCFormat::Time:
    case KCFormat::SecondeTime:
        stringFormat = cell.locale()->timeFormat();
        break;
    case KCFormat::Time1:
        stringFormat = "h:mm AM/PM";
        break;
    case KCFormat::Time2:
        stringFormat = "h:mm:ss AM/PM";
        break;
    case KCFormat::Time3:
        stringFormat = "h \"h\" mm \"min\" ss \"s\"";
        break;
    case KCFormat::Time4:
        stringFormat = "h:mm";
        break;
    case KCFormat::Time5:
        stringFormat = "h:mm:ss";
        break;
    case KCFormat::Time6:
        stringFormat = "mm:ss";
        break;
    case KCFormat::Time7:
        stringFormat = "[h]:mm:ss";
        break;
    case KCFormat::Time8:
        stringFormat = "[h]:mm";
        break;
    case KCFormat::fraction_half:
        stringFormat = "# ?/2";
        break;
    case KCFormat::fraction_quarter:
        stringFormat = "# ?/4";
        break;
    case KCFormat::fraction_eighth:
        stringFormat = "# ?/8";
        break;
    case KCFormat::fraction_sixteenth:
        stringFormat = "# ?/16";
        break;
    case KCFormat::fraction_tenth:
        stringFormat = "# ?/10";
        break;
    case KCFormat::fraction_hundredth:
        stringFormat = "# ?/100";
        break;
    case KCFormat::fraction_one_digit:
        stringFormat = "# ?/?";
        break;
    case KCFormat::fraction_two_digits:
        stringFormat = "# ?\?/?\?";
        break;
    case KCFormat::fraction_three_digits:
        stringFormat = "# ?\?\?/?\?\?";
        break;
    case KCFormat::Custom:
        stringFormat = style.customFormat();
        break;
    default:
        // This is a required parameter, so let's write a sane default
        kWarning(30521) << "Unhandled KCFormat value, setting 'General' as default: " << style.formatType();
        stringFormat = "General";
        break;
    }
    cell_style.setAttribute("KCFormat", stringFormat);

    if (hasBorder(cell, currentcolumn, currentrow))
        cell_style.appendChild(GetBorderStyle(gnumeric_doc, cell, currentcolumn, currentrow));

    return cell_style;
}


void GNUMERICExport::addAttributeItem(QDomDocument gnumeric_doc, QDomElement attributes, const QString& type, const QString& name, bool value)
{
    QDomElement gmr_attribute, gmr_type, gmr_name, gmr_value;

    gmr_attribute = gnumeric_doc.createElement("gmr:Attribute");
    attributes.appendChild(gmr_attribute);

    gmr_type = gnumeric_doc.createElement("gmr:type");
    gmr_type.appendChild(gnumeric_doc.createTextNode(type));
    gmr_attribute.appendChild(gmr_type);

    gmr_name = gnumeric_doc.createElement("gmr:name");
    gmr_name.appendChild(gnumeric_doc.createTextNode(name));
    gmr_attribute.appendChild(gmr_name);

    QString txtValue;
    if (value)
        txtValue = "true";
    else
        txtValue = "false";

    gmr_value = gnumeric_doc.createElement("gmr:value");
    gmr_value.appendChild(gnumeric_doc.createTextNode(txtValue));
    gmr_attribute.appendChild(gmr_value);
}

void GNUMERICExport::addSummaryItem(QDomDocument gnumeric_doc, QDomElement summary, const QString& name, const QString& value)
{
    if (value.isEmpty())
        return;
    QDomElement gmr_item, gmr_name, gmr_val_string;

    gmr_item = gnumeric_doc.createElement("gmr:Item");
    summary.appendChild(gmr_item);

    gmr_name = gnumeric_doc.createElement("gmr:name");
    gmr_name.appendChild(gnumeric_doc.createTextNode(name));
    gmr_item.appendChild(gmr_name);

    gmr_val_string = gnumeric_doc.createElement("gmr:val-string");
    gmr_val_string.appendChild(gnumeric_doc.createTextNode(value));
    gmr_item.appendChild(gmr_val_string);
}

// The reason why we use the KoDocument* approach and not the QDomDocument
// approach is because we don't want to export formulas but values !
KoFilter::ConversionStatus GNUMERICExport::convert(const QByteArray& from, const QByteArray& to)
{
    kDebug(30521) << "Exporting GNUmeric";

    QDomDocument gnumeric_doc = QDomDocument();

    KoDocument* document = m_chain->inputDocument();

    if (!document)
        return KoFilter::StupidError;

    if (!qobject_cast<const KCDoc *>(document)) {    // it's safer that way :)
        kWarning(30521) << "document isn't a KCDoc but a " << document->metaObject()->className();
        return KoFilter::NotImplemented;
    }
    if (to != "application/x-gnumeric" || from != "application/x-kcells") {
        kWarning(30521) << "Invalid mimetypes " << to << " " << from;
        return KoFilter::NotImplemented;
    }

    KCDoc* ksdoc = (KCDoc*)document;

    if (ksdoc->mimeType() != "application/x-kcells") {
        kWarning(30521) << "Invalid document mimetype " << ksdoc->mimeType();
        return KoFilter::NotImplemented;
    }

    /* This could be Made into a function */

    gnumeric_doc.appendChild(gnumeric_doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\""));

    QDomElement workbook = gnumeric_doc.createElement("gmr:Workbook");
    workbook.setAttribute("xmlns:gmr", "http://www.gnumeric.org/v10.dtd");
    workbook.setAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    workbook.setAttribute("xmlns:schemaLocation", "http://www.gnumeric.org/v8.xsd");
    gnumeric_doc.appendChild(workbook);

    /* End Made into a function */

    QDomElement sheets, sheet, tmp, cells, selections, cols, rows, styles, merged, margins, topMargin, leftMargin, bottomMargin, rightMargin, orientation, paper, header, footer, customSize, cellComment, objects, repeatColumns, repeatRows;

    KoDocumentInfo *DocumentInfo = document->documentInfo();

    /*
     * Attributes
     */
    QDomElement attributes = gnumeric_doc.createElement("gmr:Attributes");
    workbook.appendChild(attributes);

    addAttributeItem(gnumeric_doc, attributes, "4", "WorkbookView::show_horizontal_scrollbar", ksdoc->map()->settings()->showHorizontalScrollBar());
    addAttributeItem(gnumeric_doc, attributes, "4", "WorkbookView::show_vertical_scrollbar", ksdoc->map()->settings()->showVerticalScrollBar());
    addAttributeItem(gnumeric_doc, attributes, "4", "WorkbookView::show_notebook_tabs", ksdoc->map()->settings()->showTabBar());
    if (ksdoc->map()->settings()->completionMode() == KGlobalSettings::CompletionAuto)
        addAttributeItem(gnumeric_doc, attributes, "4", "WorkbookView::do_auto_completion", "true");
    else
        addAttributeItem(gnumeric_doc, attributes, "4", "WorkbookView::do_auto_completion", "false");
    addAttributeItem(gnumeric_doc, attributes, "4", "WorkbookView::is_protected", ksdoc->map()->isProtected());

    /*
     * Doccument summary
     */
    QDomElement summary =  gnumeric_doc.createElement("gmr:Summary");
    workbook.appendChild(summary);

    addSummaryItem(gnumeric_doc, summary, "title", DocumentInfo->aboutInfo("title"));
    addSummaryItem(gnumeric_doc, summary, "company", DocumentInfo->authorInfo("company"));
    addSummaryItem(gnumeric_doc, summary, "author", DocumentInfo->authorInfo("creator"));
    addSummaryItem(gnumeric_doc, summary, "comments", DocumentInfo->aboutInfo("comments"));
    addSummaryItem(gnumeric_doc, summary, "keywords", DocumentInfo->aboutInfo("keyword"));

    addSummaryItem(gnumeric_doc, summary, "application", "KCells");

    /*
     * Sheet name index (necessary for the gnumeric xml_sax importer)
     */
    QDomElement sheetNameIndex = gnumeric_doc.createElement("gmr:SheetNameIndex");
    workbook.appendChild(sheetNameIndex);

    foreach(KCSheet* table, ksdoc->map()->sheetList()) {
        QDomElement sheetName = gnumeric_doc.createElement("gmr:SheetName");
        sheetName.appendChild(gnumeric_doc.createTextNode(table->sheetName()));
        sheetNameIndex.appendChild(sheetName);
    }

    /*
     * Area Names
     */
    /*
    <gmr:Names>
    <gmr:Name>
      <gmr:name>test</gmr:name>
      <gmr:value>Sheet2!$A$2:$D$10</gmr:value>
      <gmr:position>A1</gmr:position>
    </gmr:Name>
    <gmr:Name>
      <gmr:name>voiture</gmr:name>
      <gmr:value>Sheet2!$A$2:$D$8</gmr:value>
      <gmr:position>A1</gmr:position>
    </gmr:Name>
    </gmr:Names>
    */
    const QList<QString> namedAreas = ksdoc->map()->namedAreaManager()->areaNames();
    if (namedAreas.count() > 0) {
        KCSheet* sheet = 0;
        QRect range;
        QDomElement areaNames = gnumeric_doc.createElement("gmr:Names");
        for (int i = 0; i < namedAreas.count(); ++i) {
            sheet = ksdoc->map()->namedAreaManager()->sheet(namedAreas[i]);
            if (!sheet)
                continue;
            range = ksdoc->map()->namedAreaManager()->namedArea(namedAreas[i]).firstRange();
            QDomElement areaName = gnumeric_doc.createElement("gmr:Name");
            QDomElement areaNameElement = gnumeric_doc.createElement("gmr:name");
            areaNameElement.appendChild(gnumeric_doc.createTextNode(namedAreas[i]));
            areaName.appendChild(areaNameElement);
            QDomElement areaNameValue = gnumeric_doc.createElement("gmr:value");
            areaNameValue.appendChild(gnumeric_doc.createTextNode(convertRefToRange(sheet->sheetName(), range)));
            areaName.appendChild(areaNameValue);
            areaNames.appendChild(areaName);
            //TODO <gmr:position>A1</gmr:position> I don't know what is it.
        }
        workbook.appendChild(areaNames);
    }


    /*
     * Sheets
     */
    sheets = gnumeric_doc.createElement("gmr:Sheets");
    workbook.appendChild(sheets);

    QString str;

    KCView * view = ksdoc->views().isEmpty() ? 0 : static_cast<KCView*>(ksdoc->views().first());
    KCCanvas * canvas = 0L;
    QString activeTableName;
    if (view) {
        canvas = view->canvasWidget();
        activeTableName =  canvas->activeSheet()->sheetName();
    }
    int i = 0;
    int indexActiveTable = 0;
    foreach(KCSheet* table, ksdoc->map()->sheetList()) {
        if (table->printSettings()->pageLayout().format == KOdfPageFormat::CustomSize) {
            customSize = gnumeric_doc.createElement("gmr:Geometry");
            customSize.setAttribute("Width", POINT_TO_MM(table->printSettings()->pageLayout().width));
            customSize.setAttribute("Height", POINT_TO_MM(table->printSettings()->pageLayout().width));
            sheets.appendChild(customSize);
            //<gmr:Geometry Width="768" Height="365"/>
        }

        sheet = gnumeric_doc.createElement("gmr:Sheet");
        sheets.appendChild(sheet);

        sheet.setAttribute("DisplayFormulas", table->getShowFormula() ? "true" : "false");
        sheet.setAttribute("HideZero", table->getHideZero() ? "true" : "false");
        sheet.setAttribute("HideGrid", !table->getShowGrid() ? "true" : "false");
        sheet.setAttribute("HideColHeader", (!ksdoc->map()->settings()->showColumnHeader() ? "true" : "false"));
        sheet.setAttribute("HideRowHeader", (!ksdoc->map()->settings()->showRowHeader() ? "true" : "false"));
        /* Not available in KCells ?
         * sheet.setAttribute("DisplayOutlines", "true");
         * sheet.setAttribute("OutlineSymbolsBelow", "true");
         * sheet.setAttribute("OutlineSymbolsRight", "true");
         * sheet.setAttribute("TabColor", "");
         * sheet.setAttribute("TabTextColor", "");
         */

        tmp = gnumeric_doc.createElement("gmr:Name");
        if (table->sheetName() == activeTableName)
            indexActiveTable = i;

        tmp.appendChild(gnumeric_doc.createTextNode(table->sheetName()));

        sheet.appendChild(tmp);

        tmp = gnumeric_doc.createElement("gmr:MaxCol");
        tmp.appendChild(gnumeric_doc.createTextNode(QString::number(table->cellStorage()->columns())));
        sheet.appendChild(tmp);

        tmp = gnumeric_doc.createElement("gmr:MaxRow");

        tmp.appendChild(gnumeric_doc.createTextNode(QString::number(table->cellStorage()->rows())));
        sheet.appendChild(tmp);

        // Zoom value doesn't appear to be correct
        // KCells 200% gives zoom() = 2.5, this in GNumeric = 250%
        tmp = gnumeric_doc.createElement("gmr:Zoom");
        if (view)
            tmp.appendChild(gnumeric_doc.createTextNode(QString::number(view->zoomHandler()->zoom())));
        else
            tmp.appendChild(gnumeric_doc.createTextNode("1.0"));
        sheet.appendChild(tmp);

        //Print Info
        tmp = gnumeric_doc.createElement("gmr:PrintInformation");
        margins = gnumeric_doc.createElement("gmr:Margins");

        topMargin = gnumeric_doc.createElement("gmr:top");
        topMargin.setAttribute("Points", table->printSettings()->pageLayout().topMargin);
        topMargin.setAttribute("PrefUnit", "mm");
        margins.appendChild(topMargin);

        bottomMargin = gnumeric_doc.createElement("gmr:bottom");
        bottomMargin.setAttribute("Points", table->printSettings()->pageLayout().bottomMargin);
        bottomMargin.setAttribute("PrefUnit", "mm");
        margins.appendChild(bottomMargin);

        leftMargin = gnumeric_doc.createElement("gmr:left");
        leftMargin.setAttribute("Points", table->printSettings()->pageLayout().leftMargin);
        leftMargin.setAttribute("PrefUnit", "mm");
        margins.appendChild(leftMargin);

        rightMargin = gnumeric_doc.createElement("gmr:right");
        rightMargin.setAttribute("Points", table->printSettings()->pageLayout().rightMargin);
        rightMargin.setAttribute("PrefUnit", "mm");
        margins.appendChild(rightMargin);

        tmp.appendChild(margins);
        sheet.appendChild(tmp);

        orientation = gnumeric_doc.createElement("gmr:orientation");
        QString orientString = table->printSettings()->pageLayout().orientation == KOdfPageFormat::Landscape ? "landscape" : "portrait";
        orientation.appendChild(gnumeric_doc.createTextNode(orientString));
        tmp.appendChild(orientation);

        //TODO for future
        //<gmr:repeat_top value="A1:IV5"/>
        //<gmr:repeat_left value="B1:D65536"/>

        int _tmpRepeatColumnStart = table->printSettings()->repeatedColumns().first;
        int _tmpRepeatColumnEnd = table->printSettings()->repeatedColumns().second;
        if (_tmpRepeatColumnStart != 0) {
            repeatColumns = gnumeric_doc.createElement("gmr:repeat_left");
            QString value = KCCell::columnName(_tmpRepeatColumnStart) + "1:" + KCCell::columnName(_tmpRepeatColumnEnd) + "65536";
            repeatColumns.setAttribute("value", value);
            tmp.appendChild(repeatColumns);
        }
        int _tmpRepeatRowStart = table->printSettings()->repeatedRows().first;
        int _tmpRepeatRowEnd = table->printSettings()->repeatedRows().second;
        if (_tmpRepeatRowStart != 0) {
            repeatRows = gnumeric_doc.createElement("gmr:repeat_top");
            QString value = 'A' + QString::number(_tmpRepeatRowStart) + ":IV" + QString::number(_tmpRepeatRowEnd);
            repeatRows.setAttribute("value", value);
            tmp.appendChild(repeatRows);
        }

        const KCHeaderFooter *const headerFooter = table->headerFooter();
        header = gnumeric_doc.createElement("gmr:Header");
        header.setAttribute("Left", convertVariable(headerFooter->headLeft()));
        header.setAttribute("Middle", convertVariable(headerFooter->headMid()));
        header.setAttribute("Right", convertVariable(headerFooter->headRight()));
        tmp.appendChild(header);

        footer = gnumeric_doc.createElement("gmr:Footer");
        footer.setAttribute("Left", convertVariable(headerFooter->footLeft()));
        footer.setAttribute("Middle", convertVariable(headerFooter->footMid()));
        footer.setAttribute("Right", convertVariable(headerFooter->footRight()));
        tmp.appendChild(footer);

        paper = gnumeric_doc.createElement("gmr:paper");
        paper.appendChild(gnumeric_doc.createTextNode(table->printSettings()->paperFormatString()));
        tmp.appendChild(paper);

        styles = gnumeric_doc.createElement("gmr:Styles");
        sheet.appendChild(styles);

        cells = gnumeric_doc.createElement("gmr:Cells");
        sheet.appendChild(cells);

        objects = gnumeric_doc.createElement("gmr:Objects");
        sheet.appendChild(objects);

        merged = gnumeric_doc.createElement("gmr:MergedRegions");
        bool mergedCells = false; // if there are no merged cells in this sheet, don't write an
        // empty mergedRegions to the file.
        // So, depending on the value of mergedCells,
        // the merged dom element is added or not.

        cols = gnumeric_doc.createElement("gmr:Cols");
        sheet.appendChild(cols);

        rows = gnumeric_doc.createElement("gmr:Rows");
        sheet.appendChild(rows);

        /*
          selections = gnumeric_doc.createElement("gmr:Selections");
          sheet.appendChild(selections);
        */
        // Ah ah ah - the document is const, but the map and table aren't. Safety: 0.
        // Either we get hold of Sheet::m_dctCells and apply the old method below
        // (for sorting) or, cleaner and already sorted, we use Sheet's API
        // (slower probably, though)
        int iMaxColumn = table->cellStorage()->columns();
        int iMaxRow = table->cellStorage()->rows();

        // this is just a bad approximation which fails for documents with less than 50 rows, but
        // we don't need any progress stuff there anyway :) (Werner)
        int value = 0;
        int step = iMaxRow > 50 ? iMaxRow / 50 : 1;
        int i = 1;

        QString emptyLines;

        /* Save selection info. */

        /* can't save selection anymore -- part of the view, not table */
        /*
          QDomElement selection = gnumeric_doc.createElement("gmr:Selection");
          QRect table_selection(table->selection());

          selections.appendChild(selection);
        */
        /*  <gmr:Selection startCol="3" startRow="2" endCol="3" endRow="2"/>*/
        /*
          selection.setAttribute("startCol", QString::number(table_selection.left()-1));
          selection.setAttribute("startRow", QString::number(table_selection.top()-1));

          selection.setAttribute("endCol", QString::number(table_selection.right()-1));
          selection.setAttribute("endRow", QString::number(table_selection.bottom()-1));
        */
        /* End selection info. */


        /* Start COLS */
        KCColumnFormat *cl = table->firstCol();
        while (cl) {
            QDomElement colinfo = gnumeric_doc.createElement("gmr:ColInfo");
            cols.appendChild(colinfo);
            colinfo.setAttribute("No", QString::number(cl->column() - 1));
            colinfo.setAttribute("Hidden", QString::number(cl->isHidden()));
            colinfo.setAttribute("Unit", QString::number(cl->width()));

            cl = cl->next();
        }

        /* End COLS */

        //  KCRowFormat *rl=table->m_cells.firstCell;
        //   <gmr:ColInfo No="1" Unit="96.75" MarginA="2" MarginB="2" HardSize="-1" Hidden="0"/>

        /* Start ROWS */
        KCRowFormat *rl = table->firstRow();
        while (rl) {
            QDomElement rowinfo = gnumeric_doc.createElement("gmr:RowInfo");
            rows.appendChild(rowinfo);
            rowinfo.setAttribute("No", QString::number(rl->row() - 1));
            rowinfo.setAttribute("Hidden", QString::number(rl->isHidden()));
            rowinfo.setAttribute("Unit", QString::number(rl->height()));

            rl = rl->next();
        }

        /* End ROWS */

        //rl->setHeight
        //  colinfo.info();
        /*
          <gmr:ColInfo No="1" Unit="96.75" MarginA="2" MarginB="2" HardSize="-1" Hidden="0"/>
          <gmr:ColInfo No="3" Unit="113.25" MarginA="2" MarginB="2" HardSize="-1"
          Hidden="0"/>
        */

        /* End COLS */

        for (int currentrow = 1; currentrow <= iMaxRow; ++currentrow, ++i) {
            if (i > step) {
                value += 2;
                emit sigProgress(value);
                i = 0;
            }

            QString line;
            for (int currentcolumn = 1; currentcolumn <= iMaxColumn; currentcolumn++) {
                QDomElement cell_contents;
                KCCell cell(table, currentcolumn, currentrow);

                QString text, style;
                QDomDocument domLink;
                QDomElement domRoot;
                QDomNode domNode;
                QDomNodeList childNodes;

                if (!cell.isDefault() && !cell.isEmpty()) {
                    if (cell.isFormula()) {
                        QString tmp = cell.userInput();
                        if (tmp.contains("=="))
                            tmp = tmp.replace("==", "=");
                        text = tmp;
                        isLink = false;
                    } else if (!cell.link().isEmpty()) {
                        isLink = true;
                        isLinkBold = false;
                        isLinkItalic = false;
                        //TODO FIXME
                        linkUrl = cell.link();
                        linkText = cell.userInput();

                    } else {
                        text = cell.userInput();
                        isLink = false;
                    }
#if 0
                    switch (cell.content()) {
                    case KCCell::Text:
                        text = cell.userInput();
                        isLink = false;
                        break;
                    case KCCell::RichText:
                        // hyperlinks
                        // Extract the cell text
                        isLink = true;
                        isLinkBold = false;
                        isLinkItalic = false;
                        domLink.setContent(cell.userInput().section("!", 1, 1));

                        domNode = domLink.firstChild();
                        domRoot = domNode.toElement();
                        text = domNode.toElement().text();

                        while (!domNode.isNull()) {
                            style = domNode.toElement().tagName();

                            if (style == "b")
                                isLinkBold = true;

                            if (style == "i")
                                isLinkItalic = true;

                            domNode = domNode.firstChild();
                        }

                        //kDebug(30521) <<"---> link, text =" << text;

                        linkUrl = domRoot.attribute("href");
                        linkText = text;

                        break;
                    case KCCell::VisualFormula:
                        isLink = false;
                        text = cell.userInput(); // untested
                        break;
                    case KCCell::KCFormula:
                        isLink = false;
                        QString tmp = cell.userInput();
                        if (tmp == "==")
                            tmp = replace("==", "=");
                        /* cell.calc( true ); // Incredible, cells are not calculated if the document was just opened text = cell.valueString(); */
                        text = tmp;
                        break;
                    }
#endif
                }

                if (!cell.isDefault()) {

                    // Check if the cell is merged
                    // Only cells with content are interesting?
                    // Otherwise it can take a while to parse a large sheet

                    if (cell.doesMergeCells()) {
                        // The cell is forced to occupy other cells
                        QDomElement merge = gnumeric_doc.createElement("gmr:Merge");

                        // Set up the range
                        QString fromCol, toCol, fromRow, toRow;
                        fromCol = cell.columnName(currentcolumn);
                        fromRow = QString::number(currentrow);
                        toCol = cell.columnName(currentcolumn + cell.mergedXCells());
                        toRow = QString::number(currentrow + cell.mergedYCells());

                        merge.appendChild(gnumeric_doc.createTextNode(fromCol + fromRow + ':' + toCol + toRow));
                        mergedCells = true;
                        merged.appendChild(merge);
                    }
                    // ---
                    if (!cell.comment().isEmpty()) {
                        //<gmr:CellComment Author="" Text="cvbcvbxcvb&#10;cb&#10;xc&#10;vbxcv&#10;" ObjectBound="A1" ObjectOffset="0 0 0 0" ObjectAnchorType="17 16 17 16" Direction="17"/>
                        cellComment = gnumeric_doc.createElement("gmr:CellComment");
                        cellComment.setAttribute("Text", cell.comment());
                        QString sCell = QString("%1%2").arg(KCCell::columnName(currentcolumn)).arg(currentrow);

                        cellComment.setAttribute("ObjectBound", sCell);
                        objects.appendChild(cellComment);

                    }
                    QDomElement gnumeric_cell = gnumeric_doc.createElement("gmr:Cell");
                    QDomElement cell_style;

                    QDomElement style_region = gnumeric_doc.createElement("gmr:StyleRegion");

                    cells.appendChild(gnumeric_cell);

                    gnumeric_cell.setAttribute("Col", QString::number(currentcolumn - 1));
                    gnumeric_cell.setAttribute("Row", QString::number(currentrow - 1));

                    /* Right now, we create a single region for each cell.. This is inefficient,
                     * but the implementation is quicker.. Probably later we will have to
                     * consolidate styles into style regions.
                     */

                    style_region.setAttribute("startCol", QString::number(currentcolumn - 1));
                    style_region.setAttribute("startRow", QString::number(currentrow - 1));
                    style_region.setAttribute("endCol", QString::number(currentcolumn - 1));
                    style_region.setAttribute("endRow", QString::number(currentrow - 1));

                    cell_style = GetCellStyle(gnumeric_doc, cell, currentcolumn, currentrow);

                    style_region.appendChild(cell_style);

                    styles.appendChild(style_region);

                    //cell_contents = gnumeric_doc.createElement("gmr:Content");
                    gnumeric_cell.appendChild(gnumeric_doc.createTextNode(text));
                    //gnumeric_cell.appendChild(cell_contents);
                }

                // Append a delimiter, but in a temp string -> if no other real cell in this line,
                // then those will be dropped
            }
        }

        if (mergedCells)
            sheet.appendChild(merged);
    }
    QDomElement uidata = gnumeric_doc.createElement("gmr:UIData");
    uidata.setAttribute("SelectedTab", indexActiveTable);
    workbook.appendChild(uidata);

    str = gnumeric_doc.toString();

    emit sigProgress(100);

    // Ok, now write to export file

    QIODevice* out = KFilterDev::deviceForFile(m_chain->outputFile(), "application/x-gzip");

    if (!out) {
        kError(30521) << "No output file! Aborting!" << endl;
        return KoFilter::FileNotFound;
    }

    if (!out->open(QIODevice::WriteOnly)) {
        kError(30521) << "Unable to open output file! Aborting!" << endl;
        delete out;
        return KoFilter::FileNotFound;
    }

    QTextStream streamOut(out);

    streamOut << str;
    streamOut.flush();

    out->close();
    delete out;

    return KoFilter::OK;
}


QString GNUMERICExport::convertRefToRange(const QString & table, const QRect & rect)
{
    QPoint topLeft(rect.topLeft());
    QPoint bottomRight(rect.bottomRight());
    if (topLeft == bottomRight)
        return convertRefToBase(table, rect);
    QString s;
    s += table;
    s += "!$";
    s += KCCell::columnName(topLeft.x());
    s += '$';
    s += QString::number(topLeft.y());
    s += ":$";
    s += KCCell::columnName(bottomRight.x());
    s += '$';
    s += QString::number(bottomRight.y());

    return s;
}


QString GNUMERICExport::convertRefToBase(const QString & table, const QRect & rect)
{
    QPoint bottomRight(rect.bottomRight());

    QString s;
    s = table;
    s += "!$";
    s += KCCell::columnName(bottomRight.x());
    s += '$';
    s += QString::number(bottomRight.y());

    return s;
}

QString GNUMERICExport::convertVariable(QString headerFooter)
{
    headerFooter = headerFooter.replace("<sheet>", "&[TAB]");
    headerFooter = headerFooter.replace("<date>", "&[DATE]");
    headerFooter = headerFooter.replace("<page>", "&[PAGE]");
    headerFooter = headerFooter.replace("<pages>", "&[PAGES]");
    headerFooter = headerFooter.replace("<time>", "&[TIME]");
    headerFooter = headerFooter.replace("<file>", "&[FILE]");

    return headerFooter;
}

#include "gnumericexport.moc"
