/* This file is part of the KDE project
   Copyright (C) 2010 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>

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
#include "ODrawClient.h"

#include <qdebug.h>
#include <QColor>
#include <KoXmlWriter.h>
#include "sheet.h"

ODrawClient::ODrawClient(MSO::OfficeArtDggContainer* dggContainer, Swinder::Sheet* sheet)
    : m_sheet(sheet)
    , m_dggContainer(dggContainer)
{
}

static qreal offset( unsigned long dimension, unsigned long offset, qreal factor ) {
    return (float)dimension * (float)offset / factor;
}

static qreal columnWidth(Swinder::Sheet* sheet, unsigned long col) {
    if( sheet->column(col, false) )
        return sheet->column(col)->width();

    return sheet->defaultColWidth();
}

static qreal rowHeight(Swinder::Sheet* sheet, unsigned long row) {
    if( sheet->row(row, false) )
        return sheet->row(row)->height();

    return sheet->defaultRowHeight();
}

QRectF ODrawClient::getRect(const MSO::OfficeArtClientAnchor& clientAnchor)
{
    const MSO::XlsOfficeArtClientAnchor* anchor = clientAnchor.anon.get<MSO::XlsOfficeArtClientAnchor>();
    if (anchor) {
        qDebug() << "colL" << anchor->colL << "colR" << anchor->colR << "dxL" << anchor->dxL << "dxR" << anchor->dxR << "rwT" << anchor->rwT << "rwB" << anchor->rwB << "dyT" << anchor->dyT << "dyB" << anchor->dyB << "fMove" << anchor->fMove << "fSize" << anchor->fSize;
        QRectF r;
        qreal colWidth = columnWidth(m_sheet, anchor->colL);
        r.setLeft(offset(colWidth, anchor->dxL, 1024));
        if (anchor->colR == anchor->colL) {
            r.setRight(offset(colWidth, anchor->dxR, 1024));
        } else {
            qreal width = colWidth - r.left();
            for (int col = anchor->colL + 1; col < anchor->colR; ++col) {
                width += columnWidth(m_sheet, col);
            }
            width += offset(columnWidth(m_sheet, anchor->colR), anchor->dxR, 1024);
            r.setWidth(width);
        }
        qreal rowHgt = rowHeight(m_sheet, anchor->rwT);
        qDebug() << "w:" << colWidth << "h:" << rowHgt;
        r.setTop(offset(rowHgt, anchor->dyT, 256));
        if (anchor->rwT == anchor->rwB) {
            r.setBottom(offset(rowHgt, anchor->dyB, 256));
        } else {
            qreal height = rowHgt - r.top();
            for (int row = anchor->rwT + 1; row < anchor->rwB; ++row) {
                height += rowHeight(m_sheet, row);
            }
            height += offset(rowHeight(m_sheet, anchor->rwB), anchor->dyB, 256);
            r.setHeight(height);
        }
        qDebug() << "rect:" << r;
        return r;
    } else {
        qDebug() << "Invalid client anchor!";
    }
    qDebug() << "NOT YET IMPLEMENTED" << __PRETTY_FUNCTION__;
    return QRectF();
}

QString ODrawClient::getPicturePath(int pib)
{
    qDebug() << "NOT YET IMPLEMENTED" << __PRETTY_FUNCTION__;
    return QString();
}

bool ODrawClient::onlyClientData(const MSO::OfficeArtClientData &o)
{
    qDebug() << "NOT YET IMPLEMENTED" << __PRETTY_FUNCTION__;
    return false;
}

void ODrawClient::processClientData(const MSO::OfficeArtClientData &o, Writer &out)
{
    qDebug() << "NOT YET IMPLEMENTED" << __PRETTY_FUNCTION__;
}

void ODrawClient::processClientTextBox(const MSO::OfficeArtClientTextBox &ct, const MSO::OfficeArtClientData *cd, Writer &out)
{
    qDebug() << "NOT YET IMPLEMENTED" << __PRETTY_FUNCTION__;
}

KoGenStyle ODrawClient::createGraphicStyle(const MSO::OfficeArtClientTextBox *ct, const MSO::OfficeArtClientData *cd, Writer &out)
{
    qDebug() << "NOT YET IMPLEMENTED" << __PRETTY_FUNCTION__;
    KoGenStyle style = KoGenStyle(KoGenStyle::GraphicAutoStyle, "graphic");
    style.setAutoStyleInStylesDotXml(out.stylesxml);
    return style;
}

void ODrawClient::addTextStyles(const MSO::OfficeArtClientTextBox *clientTextbox, const MSO::OfficeArtClientData *clientData, Writer &out, KoGenStyle &style)
{
    qDebug() << "NOT YET IMPLEMENTED" << __PRETTY_FUNCTION__;
    const QString styleName = out.styles.insert(style);
    out.xml.addAttribute("draw:style-name", styleName);
}

const MSO::OfficeArtDggContainer* ODrawClient::getOfficeArtDggContainer()
{
    return m_dggContainer;
}

QColor ODrawClient::toQColor(const MSO::OfficeArtCOLORREF &c)
{
    qDebug() << "NOT YET IMPLEMENTED" << __PRETTY_FUNCTION__;
    return QColor();
}

QString ODrawClient::formatPos(qreal v)
{
    return QString::number(v, 'f', 11) + "pt";
}
